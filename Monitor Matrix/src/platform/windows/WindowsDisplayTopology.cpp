#include "WindowsDisplayTopology.h"
#include "WindowsDisplayModes.h"

#include <QDebug>
#include <QHash>
#include <QSet>
#include <QtMath>

#include <algorithm>
#include <cmath>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

#ifdef Q_OS_WIN

struct DisplayConfigSnapshot {
    QVector<DISPLAYCONFIG_PATH_INFO> paths;
    QVector<DISPLAYCONFIG_MODE_INFO> modes;
};

bool sameLuid(const LUID& a, const LUID& b)
{
    return a.LowPart == b.LowPart && a.HighPart == b.HighPart;
}

void addResolution(QVector<DisplayResolutionOption>& list,
    const QSize& size,
    int refreshRate = 60)
{
    if (size.width() <= 0 || size.height() <= 0)
        return;

    for (DisplayResolutionOption& existing : list) {
        if (existing.size == size) {
            if (refreshRate > existing.refreshRate)
                existing.refreshRate = refreshRate;

            return;
        }
    }

    list.append({ size, refreshRate });
}

void sortResolutions(QVector<DisplayResolutionOption>& list)
{
    std::sort(list.begin(), list.end(),
        [](const DisplayResolutionOption& a,
            const DisplayResolutionOption& b) {
            const int areaA = a.size.width() * a.size.height();
            const int areaB = b.size.width() * b.size.height();

            if (areaA != areaB)
                return areaA > areaB;

            if (a.size.width() != b.size.width())
                return a.size.width() > b.size.width();

            return a.refreshRate > b.refreshRate;
        });
}

DWORD displayConfigQueryFlags()
{
    return QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
}

DWORD setDisplayConfigFlags()
{
    return SDC_USE_SUPPLIED_DISPLAY_CONFIG |
        SDC_APPLY |
        SDC_SAVE_TO_DATABASE |
        SDC_ALLOW_CHANGES |
        SDC_ALLOW_PATH_ORDER_CHANGES |
        SDC_VIRTUAL_MODE_AWARE;
}

bool queryActiveDisplayConfig(DisplayConfigSnapshot& snapshot,
    QString* errorMessage = nullptr)
{
    const DWORD flags = displayConfigQueryFlags();

    for (int attempt = 0; attempt < 5; ++attempt) {
        UINT32 pathCount = 0;
        UINT32 modeCount = 0;

        LONG result = GetDisplayConfigBufferSizes(
            flags,
            &pathCount,
            &modeCount
        );

        if (result != ERROR_SUCCESS) {
            if (errorMessage)
                *errorMessage = WindowsDisplayTopology::windowsErrorMessage(result);

            return false;
        }

        snapshot.paths.resize(static_cast<qsizetype>(pathCount));
        snapshot.modes.resize(static_cast<qsizetype>(modeCount));

        result = QueryDisplayConfig(
            flags,
            &pathCount,
            snapshot.paths.data(),
            &modeCount,
            snapshot.modes.data(),
            nullptr
        );

        if (result == ERROR_INSUFFICIENT_BUFFER)
            continue;

        if (result != ERROR_SUCCESS) {
            if (errorMessage)
                *errorMessage = WindowsDisplayTopology::windowsErrorMessage(result);

            return false;
        }

        snapshot.paths.resize(static_cast<qsizetype>(pathCount));
        snapshot.modes.resize(static_cast<qsizetype>(modeCount));
        return true;
    }

    if (errorMessage)
        *errorMessage = "Windows display topology changed while reading it.";

    return false;
}

int refreshRateFromRational(const DISPLAYCONFIG_RATIONAL& rational)
{
    if (rational.Numerator == 0 || rational.Denominator == 0)
        return 60;

    const double value =
        static_cast<double>(rational.Numerator) /
        static_cast<double>(rational.Denominator);

    return qMax(1, static_cast<int>(std::round(value)));
}

bool isValidModeIndex(UINT32 index, qsizetype modeCount)
{
    return index != DISPLAYCONFIG_PATH_MODE_IDX_INVALID &&
        index != DISPLAYCONFIG_PATH_SOURCE_MODE_IDX_INVALID &&
        index != DISPLAYCONFIG_PATH_TARGET_MODE_IDX_INVALID &&
        index < static_cast<UINT32>(modeCount);
}

int sourceModeIndexForPath(const DISPLAYCONFIG_PATH_INFO& path,
    const QVector<DISPLAYCONFIG_MODE_INFO>& modes)
{
    const UINT32 preferredIndex = path.sourceInfo.sourceModeInfoIdx;

    if (isValidModeIndex(preferredIndex, modes.size())) {
        const DISPLAYCONFIG_MODE_INFO& mode = modes[preferredIndex];

        if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE &&
            mode.id == path.sourceInfo.id &&
            sameLuid(mode.adapterId, path.sourceInfo.adapterId)) {
            return static_cast<int>(preferredIndex);
        }
    }

    for (int i = 0; i < modes.size(); ++i) {
        const DISPLAYCONFIG_MODE_INFO& mode = modes[i];

        if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE &&
            mode.id == path.sourceInfo.id &&
            sameLuid(mode.adapterId, path.sourceInfo.adapterId)) {
            return i;
        }
    }

    return -1;
}

int targetModeIndexForPath(const DISPLAYCONFIG_PATH_INFO& path,
    const QVector<DISPLAYCONFIG_MODE_INFO>& modes)
{
    const UINT32 preferredIndex = path.targetInfo.targetModeInfoIdx;

    if (isValidModeIndex(preferredIndex, modes.size())) {
        const DISPLAYCONFIG_MODE_INFO& mode = modes[preferredIndex];

        if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET &&
            mode.id == path.targetInfo.id &&
            sameLuid(mode.adapterId, path.targetInfo.adapterId)) {
            return static_cast<int>(preferredIndex);
        }
    }

    for (int i = 0; i < modes.size(); ++i) {
        const DISPLAYCONFIG_MODE_INFO& mode = modes[i];

        if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET &&
            mode.id == path.targetInfo.id &&
            sameLuid(mode.adapterId, path.targetInfo.adapterId)) {
            return i;
        }
    }

    return -1;
}

QString sourceDeviceName(const DISPLAYCONFIG_PATH_INFO& path)
{
    DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
    sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
    sourceName.header.size = sizeof(sourceName);
    sourceName.header.adapterId = path.sourceInfo.adapterId;
    sourceName.header.id = path.sourceInfo.id;

    const LONG result = DisplayConfigGetDeviceInfo(
        reinterpret_cast<DISPLAYCONFIG_DEVICE_INFO_HEADER*>(&sourceName)
    );

    if (result == ERROR_SUCCESS && sourceName.viewGdiDeviceName[0] != L'\0')
        return QString::fromWCharArray(sourceName.viewGdiDeviceName);

    return QString("%1:%2:%3")
        .arg(path.sourceInfo.adapterId.HighPart)
        .arg(path.sourceInfo.adapterId.LowPart)
        .arg(path.sourceInfo.id);
}

void addTargetModeResolution(QVector<DisplayResolutionOption>& resolutions,
    const DISPLAYCONFIG_TARGET_MODE& mode)
{
    const DISPLAYCONFIG_VIDEO_SIGNAL_INFO& signal =
        mode.targetVideoSignalInfo;

    addResolution(
        resolutions,
        QSize(
            static_cast<int>(signal.activeSize.cx),
            static_cast<int>(signal.activeSize.cy)
        ),
        refreshRateFromRational(signal.vSyncFreq)
    );
}

void addPreferredTargetResolution(
    QVector<DisplayResolutionOption>& resolutions,
    const DISPLAYCONFIG_PATH_INFO& path)
{
    DISPLAYCONFIG_TARGET_PREFERRED_MODE preferredMode = {};
    preferredMode.header.type =
        DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE;
    preferredMode.header.size = sizeof(preferredMode);
    preferredMode.header.adapterId = path.targetInfo.adapterId;
    preferredMode.header.id = path.targetInfo.id;

    const LONG result = DisplayConfigGetDeviceInfo(
        reinterpret_cast<DISPLAYCONFIG_DEVICE_INFO_HEADER*>(&preferredMode)
    );

    if (result != ERROR_SUCCESS)
        return;

    addResolution(
        resolutions,
        QSize(
            static_cast<int>(preferredMode.width),
            static_cast<int>(preferredMode.height)
        ),
        refreshRateFromRational(
            preferredMode.targetMode.targetVideoSignalInfo.vSyncFreq
        )
    );
}

QRect rectFromWinRect(const RECT& rect)
{
    return QRect(
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top
    );
}

QRect workRectForMonitorRect(const QRect& monitorRect)
{
    RECT rect = {};
    rect.left = monitorRect.x();
    rect.top = monitorRect.y();
    rect.right = monitorRect.x() + monitorRect.width();
    rect.bottom = monitorRect.y() + monitorRect.height();

    const HMONITOR monitor = MonitorFromRect(&rect, MONITOR_DEFAULTTONULL);

    if (!monitor)
        return monitorRect;

    MONITORINFO monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoW(monitor, &monitorInfo))
        return monitorRect;

    return rectFromWinRect(monitorInfo.rcWork);
}

QVector<WindowsNativeMonitor> nativeMonitorsFromSnapshot(
    const DisplayConfigSnapshot& snapshot)
{
    QVector<WindowsNativeMonitor> result;
    QSet<QString> seenSources;
    int screenIndex = 1;

    for (const DISPLAYCONFIG_PATH_INFO& path : snapshot.paths) {
        if ((path.flags & DISPLAYCONFIG_PATH_ACTIVE) == 0)
            continue;

        const int sourceIndex =
            sourceModeIndexForPath(path, snapshot.modes);

        if (sourceIndex < 0)
            continue;

        const QString deviceName = sourceDeviceName(path);

        if (seenSources.contains(deviceName))
            continue;

        seenSources.insert(deviceName);

        const DISPLAYCONFIG_SOURCE_MODE& sourceMode =
            snapshot.modes[sourceIndex].sourceMode;

        WindowsNativeMonitor info;
        info.deviceName = deviceName;
        info.displayName = QString("Screen %1").arg(screenIndex++);
        info.monitorRect = QRect(
            static_cast<int>(sourceMode.position.x),
            static_cast<int>(sourceMode.position.y),
            static_cast<int>(sourceMode.width),
            static_cast<int>(sourceMode.height)
        );
        info.workRect = workRectForMonitorRect(info.monitorRect);
        info.primary =
            sourceMode.position.x == 0 &&
            sourceMode.position.y == 0;

        addResolution(info.availableResolutions, info.monitorRect.size(), 60);
        WindowsDisplayModes::addWindowsProvidedResolutions(
            info.availableResolutions,
            deviceName
        );

        const int targetIndex =
            targetModeIndexForPath(path, snapshot.modes);

        if (targetIndex >= 0) {
            addTargetModeResolution(
                info.availableResolutions,
                snapshot.modes[targetIndex].targetMode
            );
        }

        addPreferredTargetResolution(info.availableResolutions, path);
        sortResolutions(info.availableResolutions);

        result.append(info);
    }

    return result;
}

QHash<QString, int> sourceModeIndexesByDeviceName(
    const DisplayConfigSnapshot& snapshot)
{
    QHash<QString, int> result;

    for (const DISPLAYCONFIG_PATH_INFO& path : snapshot.paths) {
        if ((path.flags & DISPLAYCONFIG_PATH_ACTIVE) == 0)
            continue;

        const QString deviceName = sourceDeviceName(path);

        if (result.contains(deviceName))
            continue;

        const int sourceIndex =
            sourceModeIndexForPath(path, snapshot.modes);

        if (sourceIndex >= 0)
            result.insert(deviceName, sourceIndex);
    }

    return result;
}

void movePathToOrderedList(QVector<DISPLAYCONFIG_PATH_INFO>& orderedPaths,
    QVector<bool>& usedPaths,
    const QVector<DISPLAYCONFIG_PATH_INFO>& paths,
    const QString& deviceName)
{
    for (int i = 0; i < paths.size(); ++i) {
        if (usedPaths[i])
            continue;

        if (sourceDeviceName(paths[i]) != deviceName)
            continue;

        orderedPaths.append(paths[i]);
        usedPaths[i] = true;
    }
}

void orderPathsByPrimarySelection(DisplayConfigSnapshot& snapshot,
    const QVector<DisplayConfigInfo>& displays,
    int primaryIndex)
{
    QVector<DISPLAYCONFIG_PATH_INFO> orderedPaths;
    QVector<bool> usedPaths(snapshot.paths.size(), false);

    if (primaryIndex >= 0 && primaryIndex < displays.size()) {
        movePathToOrderedList(
            orderedPaths,
            usedPaths,
            snapshot.paths,
            displays[primaryIndex].deviceName
        );
    }

    for (int i = 0; i < displays.size(); ++i) {
        if (i == primaryIndex)
            continue;

        movePathToOrderedList(
            orderedPaths,
            usedPaths,
            snapshot.paths,
            displays[i].deviceName
        );
    }

    for (int i = 0; i < snapshot.paths.size(); ++i) {
        if (!usedPaths[i])
            orderedPaths.append(snapshot.paths[i]);
    }

    snapshot.paths = orderedPaths;
}

#endif

} // namespace

namespace WindowsDisplayTopology {

QString windowsErrorMessage(long errorCode)
{
#ifdef Q_OS_WIN
    LPWSTR buffer = nullptr;

    const DWORD length = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        static_cast<DWORD>(errorCode),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        nullptr
    );

    if (length == 0 || !buffer)
        return QString("Windows error %1").arg(errorCode);

    QString message = QString::fromWCharArray(buffer).trimmed();
    LocalFree(buffer);

    if (message.isEmpty())
        message = QString("Windows error %1").arg(errorCode);

    return message;
#else
    return QString("Windows error %1").arg(errorCode);
#endif
}

QVector<WindowsNativeMonitor> getNativeMonitors(QString* errorMessage)
{
#ifdef Q_OS_WIN
    DisplayConfigSnapshot snapshot;

    if (!queryActiveDisplayConfig(snapshot, errorMessage))
        return {};

    return nativeMonitorsFromSnapshot(snapshot);
#else
    if (errorMessage)
        *errorMessage = "Display topology is only supported on Windows.";

    return {};
#endif
}

QList<MonitorInfo> getMonitors(QString* errorMessage)
{
    QList<MonitorInfo> result;

    const QVector<WindowsNativeMonitor> nativeMonitors =
        getNativeMonitors(errorMessage);

    for (const WindowsNativeMonitor& native : nativeMonitors) {
        MonitorInfo info;
        info.deviceName = native.deviceName;
        info.model = native.displayName;
        info.geometry = native.monitorRect;
        info.availableGeometry = native.workRect;
        info.primary = native.primary;

        result.append(info);
    }

    return result;
}

QVector<DisplayConfigInfo> getDisplayConfigs(QString* errorMessage)
{
    QVector<DisplayConfigInfo> result;

    const QVector<WindowsNativeMonitor> nativeMonitors =
        getNativeMonitors(errorMessage);

    for (const WindowsNativeMonitor& native : nativeMonitors) {
        DisplayConfigInfo info;
        info.deviceName = native.deviceName;
        info.displayName = native.displayName;
        info.geometry = native.monitorRect;
        info.isPrimary = native.primary;
        info.availableResolutions = native.availableResolutions;

        result.append(info);
    }

    return result;
}

bool resolveNativeMonitor(const MonitorInfo& monitor,
    WindowsNativeMonitor& nativeMonitor,
    QString* errorMessage)
{
    const QVector<WindowsNativeMonitor> nativeMonitors =
        getNativeMonitors(errorMessage);

    for (const WindowsNativeMonitor& native : nativeMonitors) {
        if (native.deviceName.compare(monitor.deviceName, Qt::CaseInsensitive) == 0) {
            nativeMonitor = native;
            return true;
        }
    }

    if (nativeMonitors.size() == 1) {
        nativeMonitor = nativeMonitors.first();
        return true;
    }

    for (const WindowsNativeMonitor& native : nativeMonitors) {
        if (monitor.primary && native.primary) {
            nativeMonitor = native;
            return true;
        }
    }

    if (errorMessage) {
        *errorMessage = QString("Could not resolve native monitor for %1.")
            .arg(monitor.deviceName);
    }

    return false;
}

bool applyDisplayConfigs(const QVector<DisplayConfigInfo>& displays,
    QString* errorMessage)
{
#ifdef Q_OS_WIN
    if (displays.isEmpty()) {
        if (errorMessage)
            *errorMessage = "No displays to apply.";

        return false;
    }

    return WindowsDisplayModes::applyDisplaySettingsApi(displays, errorMessage);
#else
    Q_UNUSED(displays);

    if (errorMessage)
        *errorMessage = "Display configuration is only supported on Windows.";

    return false;
#endif
}

} // namespace WindowsDisplayTopology

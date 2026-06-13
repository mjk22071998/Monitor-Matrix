#include "DisplayConfigService.h"

#include <QApplication>
#include <QScreen>
#include <QSet>
#include <algorithm>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

DisplayConfigService::DisplayConfigService(QObject* parent)
    : QObject(parent)
{
}

static void addResolution(QVector<DisplayResolutionOption>& list,
    const QSize& size,
    int refreshRate = 60)
{
    if (!size.isValid())
        return;

    for (auto& existing : list) {
        if (existing.size == size) {
            if (refreshRate > existing.refreshRate)
                existing.refreshRate = refreshRate;
            return;
        }
    }

    list.append({ size, refreshRate });
}

static void sortResolutions(QVector<DisplayResolutionOption>& list)
{
    std::sort(list.begin(), list.end(),
        [](const DisplayResolutionOption& a, const DisplayResolutionOption& b) {
            const int areaA = a.size.width() * a.size.height();
            const int areaB = b.size.width() * b.size.height();

            if (areaA != areaB)
                return areaA > areaB;

            if (a.size.width() != b.size.width())
                return a.size.width() > b.size.width();

            return a.refreshRate > b.refreshRate;
        });
}

#ifdef Q_OS_WIN
static QVector<DisplayResolutionOption> getAvailableResolutionsForDevice(const QString& deviceName)
{
    QVector<DisplayResolutionOption> result;

    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    DWORD modeIndex = 0;

    while (EnumDisplaySettings(
        reinterpret_cast<LPCSTR>(deviceName.toStdString().c_str()),
        modeIndex,
        &devMode
    )) {
        const QSize size(
            static_cast<int>(devMode.dmPelsWidth),
            static_cast<int>(devMode.dmPelsHeight)
        );

        const int refreshRate =
            devMode.dmDisplayFrequency > 0
            ? static_cast<int>(devMode.dmDisplayFrequency)
            : 60;

        addResolution(result, size, refreshRate);

        ZeroMemory(&devMode, sizeof(devMode));
        devMode.dmSize = sizeof(devMode);
        ++modeIndex;
    }

    sortResolutions(result);
    return result;
}

static QVector<DisplayConfigInfo> getWindowsDisplays()
{
    QVector<DisplayConfigInfo> result;

    DISPLAY_DEVICE displayDevice;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    DWORD deviceIndex = 0;
    int screenIndex = 1;

    while (EnumDisplayDevices(nullptr, deviceIndex, &displayDevice, 0)) {
        const bool attached =
            (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP);

        const bool mirror =
            (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER);

        if (attached && !mirror) {
            const QString deviceName =
                QString(displayDevice.DeviceName);

            DEVMODE currentMode;
            ZeroMemory(&currentMode, sizeof(currentMode));
            currentMode.dmSize = sizeof(currentMode);

            if (EnumDisplaySettings(
                displayDevice.DeviceName,
                ENUM_CURRENT_SETTINGS,
                &currentMode
            )) {
                DisplayConfigInfo info;

                info.deviceName = deviceName;
                info.displayName = QString("Screen %1").arg(screenIndex++);
                info.geometry = QRect(
                    static_cast<int>(currentMode.dmPosition.x),
                    static_cast<int>(currentMode.dmPosition.y),
                    static_cast<int>(currentMode.dmPelsWidth),
                    static_cast<int>(currentMode.dmPelsHeight)
                );

                info.isPrimary =
                    (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);

                info.availableResolutions =
                    getAvailableResolutionsForDevice(deviceName);

                addResolution(
                    info.availableResolutions,
                    info.geometry.size(),
                    currentMode.dmDisplayFrequency > 0
                    ? static_cast<int>(currentMode.dmDisplayFrequency)
                    : 60
                );

                sortResolutions(info.availableResolutions);

                result.append(info);
            }
        }

        ZeroMemory(&displayDevice, sizeof(displayDevice));
        displayDevice.cb = sizeof(displayDevice);
        ++deviceIndex;
    }

    return result;
}
#endif

QVector<DisplayConfigInfo> DisplayConfigService::getDisplays() const
{
#ifdef Q_OS_WIN
    QVector<DisplayConfigInfo> windowsResult = getWindowsDisplays();

    if (!windowsResult.isEmpty()) {
        return windowsResult;
    }
#endif

    QVector<DisplayConfigInfo> qtResult;

    const QList<QScreen*> screens = QApplication::screens();
    QScreen* primary = QApplication::primaryScreen();

    int index = 1;

    for (QScreen* screen : screens) {
        DisplayConfigInfo info;

        info.deviceName = screen->name();
        info.displayName = QString("Screen %1").arg(index++);
        info.geometry = screen->geometry();
        info.isPrimary = (screen == primary);

        addResolution(info.availableResolutions, info.geometry.size(), 60);

        qtResult.append(info);
    }

    return qtResult;
}

bool DisplayConfigService::applyDisplays(const QVector<DisplayConfigInfo>& displays)
{
#ifdef Q_OS_WIN
    if (displays.isEmpty()) {
        emit applyFailed("No displays to apply.");
        return false;
    }

    int primaryIndex = -1;

    for (int i = 0; i < displays.size(); ++i) {
        if (displays[i].isPrimary) {
            primaryIndex = i;
            break;
        }
    }

    if (primaryIndex < 0)
        primaryIndex = 0;

    const QPoint primaryOffset = displays[primaryIndex].geometry.topLeft();

    for (int i = 0; i < displays.size(); ++i) {
        const DisplayConfigInfo& display = displays[i];

        QByteArray deviceNameBytes = display.deviceName.toLocal8Bit();
        const char* deviceName = deviceNameBytes.constData();

        DEVMODE devMode;
        ZeroMemory(&devMode, sizeof(devMode));
        devMode.dmSize = sizeof(devMode);

        if (!EnumDisplaySettingsA(deviceName, ENUM_CURRENT_SETTINGS, &devMode)) {
            emit applyFailed(QString("Could not read settings for %1.")
                .arg(display.displayName));
            return false;
        }

        const QRect geometry = display.geometry.translated(-primaryOffset);

        devMode.dmPosition.x = geometry.x();
        devMode.dmPosition.y = geometry.y();
        devMode.dmPelsWidth = geometry.width();
        devMode.dmPelsHeight = geometry.height();

        devMode.dmFields =
            DM_POSITION |
            DM_PELSWIDTH |
            DM_PELSHEIGHT;

        DWORD flags = CDS_UPDATEREGISTRY | CDS_NORESET;

        if (i == primaryIndex)
            flags |= CDS_SET_PRIMARY;

        const LONG result = ChangeDisplaySettingsExA(
            deviceName,
            &devMode,
            nullptr,
            flags,
            nullptr
        );

        if (result != DISP_CHANGE_SUCCESSFUL) {
            emit applyFailed(QString("Could not apply settings for %1.")
                .arg(display.displayName));
            return false;
        }
    }

    const LONG finalResult = ChangeDisplaySettingsExA(
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr
    );

    if (finalResult != DISP_CHANGE_SUCCESSFUL) {
        emit applyFailed("Windows rejected the final display layout.");
        return false;
    }

    emit displaysApplied();
    return true;
#else
    Q_UNUSED(displays);
    emit applyFailed("Display configuration is only supported on Windows.");
    return false;
#endif
}
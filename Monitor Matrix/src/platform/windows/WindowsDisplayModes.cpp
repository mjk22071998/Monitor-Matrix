#include "WindowsDisplayModes.h"

#include <QDebug>

#include <algorithm>
#include <limits>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

#ifdef Q_OS_WIN

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

QString displayChangeResultMessage(LONG result)
{
    switch (result) {
    case DISP_CHANGE_SUCCESSFUL:
        return QStringLiteral("The display settings were changed successfully.");
    case DISP_CHANGE_RESTART:
        return QStringLiteral("Windows accepted the display settings but requires a restart.");
    case DISP_CHANGE_BADDUALVIEW:
        return QStringLiteral("Windows rejected the settings because DualView is active.");
    case DISP_CHANGE_BADFLAGS:
        return QStringLiteral("Windows rejected invalid display-change flags.");
    case DISP_CHANGE_BADMODE:
        return QStringLiteral("Windows rejected this display mode.");
    case DISP_CHANGE_BADPARAM:
        return QStringLiteral("Windows rejected invalid display-change parameters.");
    case DISP_CHANGE_FAILED:
        return QStringLiteral("The display driver failed the display change.");
    case DISP_CHANGE_NOTUPDATED:
        return QStringLiteral("Windows could not write the display settings to the registry.");
    default:
        return QStringLiteral("Windows display-change error %1.").arg(result);
    }
}

int refreshRateFromDevMode(const DEVMODEW& mode)
{
    if ((mode.dmFields & DM_DISPLAYFREQUENCY) == 0 ||
        mode.dmDisplayFrequency <= 1) {
        return 60;
    }

    return static_cast<int>(mode.dmDisplayFrequency);
}

int bitsPerPixelFromDevMode(const DEVMODEW& mode)
{
    if ((mode.dmFields & DM_BITSPERPEL) == 0)
        return 0;

    return static_cast<int>(mode.dmBitsPerPel);
}

bool getCurrentDisplayMode(const QString& deviceName, DEVMODEW& currentMode)
{
    currentMode = {};
    currentMode.dmSize = sizeof(currentMode);

    return EnumDisplaySettingsExW(
        reinterpret_cast<LPCWSTR>(deviceName.utf16()),
        ENUM_CURRENT_SETTINGS,
        &currentMode,
        0
    );
}

int addDisplaySettingsResolutions(
    QVector<DisplayResolutionOption>& resolutions,
    const QString& deviceName,
    DWORD flags)
{
    int addedCount = 0;

    for (DWORD modeIndex = 0;; ++modeIndex) {
        DEVMODEW mode = {};
        mode.dmSize = sizeof(mode);

        const BOOL ok = EnumDisplaySettingsExW(
            reinterpret_cast<LPCWSTR>(deviceName.utf16()),
            modeIndex,
            &mode,
            flags
        );

        if (!ok)
            break;

        if ((mode.dmFields & DM_PELSWIDTH) == 0 ||
            (mode.dmFields & DM_PELSHEIGHT) == 0) {
            continue;
        }

        const int width = static_cast<int>(mode.dmPelsWidth);
        const int height = static_cast<int>(mode.dmPelsHeight);

        if (width <= 0 || height <= 0)
            continue;

        int refreshRate = 60;

        if ((mode.dmFields & DM_DISPLAYFREQUENCY) != 0 &&
            mode.dmDisplayFrequency > 1) {
            refreshRate = static_cast<int>(mode.dmDisplayFrequency);
        }

        const int before = resolutions.size();
        addResolution(resolutions, QSize(width, height), refreshRate);

        if (resolutions.size() > before)
            ++addedCount;
    }

    return addedCount;
}

bool findDisplayModeForSizeWithFlags(
    const QString& deviceName,
    const QSize& size,
    DWORD flags,
    const DEVMODEW& currentMode,
    DEVMODEW& selectedMode)
{
    bool found = false;
    int bestScore = std::numeric_limits<int>::min();
    const int currentRefreshRate = refreshRateFromDevMode(currentMode);
    const int currentBitsPerPixel = bitsPerPixelFromDevMode(currentMode);

    for (DWORD modeIndex = 0;; ++modeIndex) {
        DEVMODEW mode = {};
        mode.dmSize = sizeof(mode);

        const BOOL ok = EnumDisplaySettingsExW(
            reinterpret_cast<LPCWSTR>(deviceName.utf16()),
            modeIndex,
            &mode,
            flags
        );

        if (!ok)
            break;

        if ((mode.dmFields & DM_PELSWIDTH) == 0 ||
            (mode.dmFields & DM_PELSHEIGHT) == 0) {
            continue;
        }

        if (static_cast<int>(mode.dmPelsWidth) != size.width() ||
            static_cast<int>(mode.dmPelsHeight) != size.height()) {
            continue;
        }

        const int refreshRate = refreshRateFromDevMode(mode);
        const int bitsPerPixel = bitsPerPixelFromDevMode(mode);
        int score = refreshRate + bitsPerPixel;

        if (refreshRate == currentRefreshRate)
            score += 100000;

        if (bitsPerPixel == currentBitsPerPixel)
            score += 10000;

        if (!found || score > bestScore) {
            selectedMode = mode;
            bestScore = score;
            found = true;
        }
    }

    return found;
}

bool findDisplayModeForSize(
    const QString& deviceName,
    const QSize& size,
    DEVMODEW& selectedMode,
    QString* errorMessage)
{
    DEVMODEW currentMode = {};

    if (!getCurrentDisplayMode(deviceName, currentMode)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Could not read current display settings for %1.")
                .arg(deviceName);
        }

        return false;
    }

    if (findDisplayModeForSizeWithFlags(
        deviceName,
        size,
        0,
        currentMode,
        selectedMode)) {
        return true;
    }

    if (findDisplayModeForSizeWithFlags(
        deviceName,
        size,
        EDS_RAWMODE,
        currentMode,
        selectedMode)) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Windows does not expose %1 x %2 for %3 anymore.")
            .arg(size.width())
            .arg(size.height())
            .arg(deviceName);
    }

    return false;
}

bool testDisplayMode(
    const QString& deviceName,
    DEVMODEW& mode,
    DWORD flags,
    QString* errorMessage)
{
    const LONG result = ChangeDisplaySettingsExW(
        reinterpret_cast<LPCWSTR>(deviceName.utf16()),
        &mode,
        nullptr,
        flags | CDS_TEST,
        nullptr
    );

    if (result == DISP_CHANGE_SUCCESSFUL)
        return true;

    if (errorMessage) {
        *errorMessage = QStringLiteral("Windows rejected test settings for %1. %2")
            .arg(deviceName, displayChangeResultMessage(result));
    }

    return false;
}

#endif

}

namespace WindowsDisplayModes {

void addWindowsProvidedResolutions(
    QVector<DisplayResolutionOption>& resolutions,
    const QString& deviceName)
{
#ifdef Q_OS_WIN
    const int standardCount =
        addDisplaySettingsResolutions(resolutions, deviceName, 0);

    qInfo().noquote()
        << "Display settings resolutions enumerated"
        << "device="
        << deviceName
        << "count="
        << standardCount;

    if (standardCount > 1)
        return;

    const int rawCount =
        addDisplaySettingsResolutions(resolutions, deviceName, EDS_RAWMODE);

    qInfo().noquote()
        << "Raw display settings resolutions enumerated"
        << "device="
        << deviceName
        << "count="
        << rawCount;
#else
    Q_UNUSED(resolutions);
    Q_UNUSED(deviceName);
#endif
}

bool applyDisplaySettingsApi(
    const QVector<DisplayConfigInfo>& displays,
    QString* errorMessage)
{
#ifdef Q_OS_WIN
    struct PendingChange {
        QString deviceName;
        QString displayName;
        DEVMODEW mode = {};
        DWORD flags = 0;
    };

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
    QVector<PendingChange> pendingChanges;

    for (int i = 0; i < displays.size(); ++i) {
        const DisplayConfigInfo& display = displays[i];
        DEVMODEW mode = {};

        if (!findDisplayModeForSize(
            display.deviceName,
            display.geometry.size(),
            mode,
            errorMessage)) {
            return false;
        }

        const QRect geometry = display.geometry.translated(-primaryOffset);

        mode.dmSize = sizeof(mode);
        mode.dmFields |= DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT;
        mode.dmPosition.x = geometry.x();
        mode.dmPosition.y = geometry.y();
        mode.dmPelsWidth = static_cast<DWORD>(geometry.width());
        mode.dmPelsHeight = static_cast<DWORD>(geometry.height());

        DWORD flags = CDS_UPDATEREGISTRY | CDS_NORESET;

        if (i == primaryIndex)
            flags |= CDS_SET_PRIMARY;

        qInfo().noquote()
            << "Prepared display settings"
            << "display="
            << display.displayName
            << "device="
            << display.deviceName
            << "x="
            << geometry.x()
            << "y="
            << geometry.y()
            << "w="
            << geometry.width()
            << "h="
            << geometry.height()
            << "primary="
            << (i == primaryIndex);

        PendingChange change;
        change.deviceName = display.deviceName;
        change.displayName = display.displayName;
        change.mode = mode;
        change.flags = flags;
        pendingChanges.append(change);
    }

    for (PendingChange& change : pendingChanges) {
        if (!testDisplayMode(
            change.deviceName,
            change.mode,
            change.flags,
            errorMessage)) {
            return false;
        }
    }

    for (PendingChange& change : pendingChanges) {
        const LONG result = ChangeDisplaySettingsExW(
            reinterpret_cast<LPCWSTR>(change.deviceName.utf16()),
            &change.mode,
            nullptr,
            change.flags,
            nullptr
        );

        if (result != DISP_CHANGE_SUCCESSFUL) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Windows rejected settings for %1. %2")
                    .arg(change.displayName, displayChangeResultMessage(result));
            }

            return false;
        }
    }

    const LONG result = ChangeDisplaySettingsExW(
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr
    );

    if (result != DISP_CHANGE_SUCCESSFUL) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Windows rejected the final display apply. %1")
                .arg(displayChangeResultMessage(result));
        }

        return false;
    }

    return true;
#else
    Q_UNUSED(displays);

    if (errorMessage)
        *errorMessage = "Display configuration is only supported on Windows.";

    return false;
#endif
}

}

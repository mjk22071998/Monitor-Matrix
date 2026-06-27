#pragma once

#include <QList>
#include <QRect>
#include <QString>
#include <QVector>

#include "../../models/DisplayConfig.h"
#include "../../models/MonitorInfo.h"

struct WindowsNativeMonitor {
    QString deviceName;
    QString displayName;
    QRect monitorRect;
    QRect workRect;
    bool primary = false;
    QVector<DisplayResolutionOption> availableResolutions;
};

namespace WindowsDisplayTopology {
QVector<WindowsNativeMonitor> getNativeMonitors(QString* errorMessage = nullptr);
QList<MonitorInfo> getMonitors(QString* errorMessage = nullptr);
QVector<DisplayConfigInfo> getDisplayConfigs(QString* errorMessage = nullptr);
bool applyDisplayConfigs(const QVector<DisplayConfigInfo>& displays,
    QString* errorMessage = nullptr);
bool resolveNativeMonitor(const MonitorInfo& monitor,
    WindowsNativeMonitor& nativeMonitor,
    QString* errorMessage = nullptr);
QString windowsErrorMessage(long errorCode);
}

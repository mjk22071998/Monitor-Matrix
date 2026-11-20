#include "MonitorCanvasModel.h"

#include <QGuiApplication>
#include <QScreen>

MonitorCanvasModel::MonitorCanvasModel(MonitorService* service, QObject* parent)
    : QAbstractListModel(parent)
{
    if (service) {
        setMonitorService(service);
    }
}

int MonitorCanvasModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_monitors.size();
}

QVariant MonitorCanvasModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_monitors.size()) {
        return QVariant();
    }

    const MonitorInfo& info = m_monitors.at(index.row());

    switch (role) {
    case DeviceNameRole:
        return info.deviceName;
    case ModelNameRole:
        return info.model;
    case RectXRole:
        return static_cast<qreal>(info.geometry.x());
    case RectYRole:
        return static_cast<qreal>(info.geometry.y());
    case RectWidthRole:
        return static_cast<qreal>(info.geometry.width());
    case RectHeightRole:
        return static_cast<qreal>(info.geometry.height());
    case AvailXRole:
        return static_cast<qreal>(info.availableGeometry.x());
    case AvailYRole:
        return static_cast<qreal>(info.availableGeometry.y());
    case AvailWidthRole:
        return static_cast<qreal>(info.availableGeometry.width());
    case AvailHeightRole:
        return static_cast<qreal>(info.availableGeometry.height());
    case IsPrimaryRole:
        return info.primary;
    case ScaleFactorRole:
        return computeScaleFactor(info);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MonitorCanvasModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DeviceNameRole] = "deviceName";
    roles[ModelNameRole] = "modelName";
    roles[RectXRole] = "rectX";
    roles[RectYRole] = "rectY";
    roles[RectWidthRole] = "rectWidth";
    roles[RectHeightRole] = "rectHeight";
    roles[AvailXRole] = "availX";
    roles[AvailYRole] = "availY";
    roles[AvailWidthRole] = "availWidth";
    roles[AvailHeightRole] = "availHeight";
    roles[IsPrimaryRole] = "isPrimary";
    roles[ScaleFactorRole] = "scaleFactor";
    return roles;
}

void MonitorCanvasModel::setMonitorService(MonitorService* service)
{
    if (m_monitorService) {
        disconnect(m_monitorService, &MonitorService::monitorsChanged, this, &MonitorCanvasModel::refreshMonitors);
    }

    m_monitorService = service;

    if (m_monitorService) {
        connect(m_monitorService, &MonitorService::monitorsChanged, this, &MonitorCanvasModel::refreshMonitors);
        refreshMonitors();
    }
}

void MonitorCanvasModel::setMonitors(const QList<MonitorInfo>& monitors)
{
    beginResetModel();
    m_monitors = monitors;
    endResetModel();
    emit monitorsChanged();
}

void MonitorCanvasModel::refreshMonitors()
{
    if (m_monitorService) {
        m_monitorService->refreshMonitors();
        setMonitors(m_monitorService->getCurrentMonitors());
    }
}

QVariantMap MonitorCanvasModel::getMonitor(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_monitors.size()) {
        return map;
    }

    const MonitorInfo& info = m_monitors.at(index);
    map[QStringLiteral("deviceName")] = info.deviceName;
    map[QStringLiteral("modelName")] = info.model;
    map[QStringLiteral("rectX")] = static_cast<qreal>(info.geometry.x());
    map[QStringLiteral("rectY")] = static_cast<qreal>(info.geometry.y());
    map[QStringLiteral("rectWidth")] = static_cast<qreal>(info.geometry.width());
    map[QStringLiteral("rectHeight")] = static_cast<qreal>(info.geometry.height());
    map[QStringLiteral("availX")] = static_cast<qreal>(info.availableGeometry.x());
    map[QStringLiteral("availY")] = static_cast<qreal>(info.availableGeometry.y());
    map[QStringLiteral("availWidth")] = static_cast<qreal>(info.availableGeometry.width());
    map[QStringLiteral("availHeight")] = static_cast<qreal>(info.availableGeometry.height());
    map[QStringLiteral("isPrimary")] = info.primary;
    map[QStringLiteral("scaleFactor")] = computeScaleFactor(info);
    return map;
}

QVariantMap MonitorCanvasModel::getMonitorByName(const QString& deviceName) const
{
    for (int i = 0; i < m_monitors.size(); ++i) {
        if (m_monitors.at(i).deviceName == deviceName) {
            return getMonitor(i);
        }
    }
    return QVariantMap();
}

qreal MonitorCanvasModel::computeScaleFactor(const MonitorInfo& monitor) const
{
    const auto screens = QGuiApplication::screens();
    for (const QScreen* screen : screens) {
        if (!screen) {
            continue;
        }
        if (screen->name() == monitor.deviceName || screen->geometry() == monitor.geometry) {
            return screen->devicePixelRatio();
        }
    }
    return 1.0;
}

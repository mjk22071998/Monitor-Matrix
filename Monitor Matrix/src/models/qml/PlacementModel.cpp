#include "PlacementModel.h"

#include <algorithm>
#include <QUuid>
#include <QVector>

PlacementModel::PlacementModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int PlacementModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_placements.size();
}

QVariant PlacementModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_placements.size()) {
        return QVariant();
    }

    const PlacementItem& item = m_placements.at(index.row());

    switch (role) {
    case PlacementIdRole:
        return item.placementId;
    case AppIdRole:
        return item.appId;
    case AppNameRole:
        return item.appName;
    case AppTypeRole:
        return item.appType;
    case TargetDeviceNameRole:
        return item.targetDeviceName;
    case RectXRole:
        return item.rect.x();
    case RectYRole:
        return item.rect.y();
    case RectWRole:
        return item.rect.width();
    case RectHRole:
        return item.rect.height();
    case PositionEnumRole:
        return item.positionEnum;
    case HasOverlapRole:
        return item.hasOverlap;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> PlacementModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PlacementIdRole] = "placementId";
    roles[AppIdRole] = "appId";
    roles[AppNameRole] = "appName";
    roles[AppTypeRole] = "appType";
    roles[TargetDeviceNameRole] = "targetDeviceName";
    roles[RectXRole] = "rectX";
    roles[RectYRole] = "rectY";
    roles[RectWRole] = "rectW";
    roles[RectHRole] = "rectH";
    roles[PositionEnumRole] = "positionEnum";
    roles[HasOverlapRole] = "hasOverlap";
    return roles;
}

QString PlacementModel::placeApp(
    const QString& appId,
    const QString& appName,
    const QString& targetDeviceName,
    int positionEnum,
    const QRectF& rect,
    const QString& appType)
{
    PlacementItem item;
    item.placementId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.appId = appId;
    item.appName = appName;
    item.appType = appType;
    item.targetDeviceName = targetDeviceName;
    item.positionEnum = positionEnum;

    QRectF targetRect = rect;
    if (targetRect.isEmpty()) {
        targetRect = QRectF(0, 0, 400, 300);
    }
    item.rect = clampToConstraints(targetDeviceName, targetRect);

    const int newRow = m_placements.size();
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_placements.append(item);
    endInsertRows();

    recomputeOverlaps();

    emit countChanged();
    emit placementsChanged();

    return item.placementId;
}

bool PlacementModel::updatePlacementRect(const QString& placementId, const QRectF& newRect)
{
    for (int i = 0; i < m_placements.size(); ++i) {
        if (m_placements.at(i).placementId == placementId) {
            PlacementItem& item = m_placements[i];
            item.rect = clampToConstraints(item.targetDeviceName, newRect);

            emit dataChanged(
                index(i, 0),
                index(i, 0),
                { RectXRole, RectYRole, RectWRole, RectHRole }
            );

            recomputeOverlaps();
            emit placementsChanged();
            return true;
        }
    }
    return false;
}

QRectF PlacementModel::placementRect(const QString& placementId) const
{
    for (const PlacementItem& item : m_placements) {
        if (item.placementId == placementId) {
            return item.rect;
        }
    }
    return QRectF();
}

bool PlacementModel::removePlacement(const QString& placementId)
{
    for (int i = 0; i < m_placements.size(); ++i) {
        if (m_placements.at(i).placementId == placementId) {
            beginRemoveRows(QModelIndex(), i, i);
            m_placements.removeAt(i);
            endRemoveRows();

            recomputeOverlaps();

            emit countChanged();
            emit placementsChanged();
            return true;
        }
    }
    return false;
}

void PlacementModel::clearAll()
{
    if (m_placements.isEmpty()) {
        return;
    }

    beginResetModel();
    m_placements.clear();
    endResetModel();

    recomputeOverlaps();

    emit countChanged();
    emit placementsChanged();
}

QVariantMap PlacementModel::get(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_placements.size()) {
        return map;
    }

    const PlacementItem& item = m_placements.at(index);
    map[QStringLiteral("placementId")] = item.placementId;
    map[QStringLiteral("appId")] = item.appId;
    map[QStringLiteral("appName")] = item.appName;
    map[QStringLiteral("appType")] = item.appType;
    map[QStringLiteral("targetDeviceName")] = item.targetDeviceName;
    map[QStringLiteral("rectX")] = item.rect.x();
    map[QStringLiteral("rectY")] = item.rect.y();
    map[QStringLiteral("rectW")] = item.rect.width();
    map[QStringLiteral("rectH")] = item.rect.height();
    map[QStringLiteral("positionEnum")] = item.positionEnum;
    map[QStringLiteral("hasOverlap")] = item.hasOverlap;
    return map;
}

QVariantMap PlacementModel::getById(const QString& placementId) const
{
    for (int i = 0; i < m_placements.size(); ++i) {
        if (m_placements.at(i).placementId == placementId) {
            return get(i);
        }
    }
    return QVariantMap();
}

QStringList PlacementModel::placedAppIds() const
{
    QStringList ids;
    for (const PlacementItem& item : m_placements) {
        if (!ids.contains(item.appId)) {
            ids.append(item.appId);
        }
    }
    return ids;
}

void PlacementModel::setMonitorBounds(const QString& deviceName, const QRectF& bounds)
{
    m_monitorBounds[deviceName] = bounds;
}

void PlacementModel::setMonitors(const QList<MonitorInfo>& monitors)
{
    m_monitors = monitors;
    m_monitorBounds.clear();
    for (const MonitorInfo& monitor : monitors) {
        m_monitorBounds[monitor.deviceName] = QRectF(
            0,
            0,
            static_cast<qreal>(monitor.geometry.width()),
            static_cast<qreal>(monitor.geometry.height())
        );
    }
}

QList<Placement> PlacementModel::toPlacements() const
{
    QList<Placement> result;
    for (const PlacementItem& item : m_placements) {
        MonitorInfo mon;
        mon.deviceName = item.targetDeviceName;
        for (const MonitorInfo& m : m_monitors) {
            if (m.deviceName.compare(item.targetDeviceName, Qt::CaseInsensitive) == 0) {
                mon = m;
                break;
            }
        }
        if (mon.geometry.isNull()) {
            for (auto it = m_monitorBounds.constBegin(); it != m_monitorBounds.constEnd(); ++it) {
                if (it.key().compare(item.targetDeviceName, Qt::CaseInsensitive) == 0) {
                    mon.geometry = it.value().toRect();
                    mon.availableGeometry = mon.geometry;
                    break;
                }
            }
        }

        QRect zone = item.rect.toRect();

        Placement p(item.appId, mon, zone);
        p.placementId = item.placementId;
        p.position = static_cast<AppPosition>(item.positionEnum);
        result.append(p);
    }
    return result;
}

void PlacementModel::loadPlacements(const QList<Placement>& placements, const QList<AppItem>& catalog)
{
    beginResetModel();
    m_placements.clear();

    for (const Placement& p : placements) {
        PlacementItem item;
        item.placementId = p.placementId.isEmpty()
            ? QUuid::createUuid().toString(QUuid::WithoutBraces)
            : p.placementId;
        item.appId = p.appId;
        item.targetDeviceName = p.monitor.deviceName;
        item.positionEnum = static_cast<int>(p.position);

        // Find app name and type from catalog if available
        auto it = std::find_if(catalog.begin(), catalog.end(), [&](const AppItem& app) {
            return app.id == p.appId;
        });
        if (it != catalog.end()) {
            item.appName = it->name;
            item.appType = it->type;
        } else {
            item.appName = p.appId;
            item.appType = QStringLiteral("Application");
        }

        QRectF rect(p.zoneRect);
        if (rect.isEmpty()) {
            rect = QRectF(0, 0, 400, 300);
        }
        item.rect = clampToConstraints(item.targetDeviceName, rect);
        m_placements.append(item);
    }

    endResetModel();

    recomputeOverlaps();

    emit countChanged();
    emit placementsChanged();
}

void PlacementModel::recomputeOverlaps()
{
    const int n = m_placements.size();
    QVector<bool> overlaps(n, false);
    int count = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (m_placements[i].targetDeviceName.compare(m_placements[j].targetDeviceName, Qt::CaseInsensitive) == 0) {
                if (m_placements[i].rect.intersects(m_placements[j].rect)) {
                    overlaps[i] = true;
                    overlaps[j] = true;
                }
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        if (overlaps[i]) {
            ++count;
        }
        if (m_placements[i].hasOverlap != overlaps[i]) {
            m_placements[i].hasOverlap = overlaps[i];
            emit dataChanged(index(i, 0), index(i, 0), { HasOverlapRole });
        }
    }

    const bool hasAny = (count > 0);
    if (m_hasOverlaps != hasAny || m_invalidCount != count) {
        m_hasOverlaps = hasAny;
        m_invalidCount = count;
        emit overlapStateChanged();
    }
}

QRectF PlacementModel::clampToConstraints(const QString& deviceName, const QRectF& rect) const
{
    qreal width = std::max(MIN_SIZE, rect.width());
    qreal height = std::max(MIN_SIZE, rect.height());
    qreal x = rect.x();
    qreal y = rect.y();

    QRectF bounds;
    for (auto it = m_monitorBounds.constBegin(); it != m_monitorBounds.constEnd(); ++it) {
        if (it.key().compare(deviceName, Qt::CaseInsensitive) == 0) {
            bounds = it.value();
            break;
        }
    }

    if (bounds.width() > 0 && bounds.height() > 0) {
        if (width > bounds.width()) {
            width = bounds.width();
        }
        if (height > bounds.height()) {
            height = bounds.height();
        }
        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }
        if (x + width > bounds.width()) {
            x = bounds.width() - width;
        }
        if (y + height > bounds.height()) {
            y = bounds.height() - height;
        }
    }

    return QRectF(x, y, width, height);
}

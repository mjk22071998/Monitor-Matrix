#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "../Placement.h"
#include "../AppItem.h"
#include "../MonitorInfo.h"

struct PlacementItem {
    QString placementId;
    QString appId;
    QString appName;
    QString appType;
    QString targetDeviceName;
    QRectF rect;
    int positionEnum = 0;
    bool hasOverlap = false;
};

class PlacementModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool hasOverlaps READ hasOverlaps NOTIFY overlapStateChanged)
    Q_PROPERTY(int invalidCount READ invalidCount NOTIFY overlapStateChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    static constexpr qreal MIN_SIZE = 40.0;

    enum Roles {
        PlacementIdRole = Qt::UserRole + 1,
        AppIdRole,
        AppNameRole,
        AppTypeRole,
        TargetDeviceNameRole,
        RectXRole,
        RectYRole,
        RectWRole,
        RectHRole,
        PositionEnumRole,
        HasOverlapRole
    };
    Q_ENUM(Roles)

    explicit PlacementModel(QObject* parent = nullptr);
    ~PlacementModel() override = default;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_placements.size(); }
    bool hasOverlaps() const { return m_hasOverlaps; }
    int invalidCount() const { return m_invalidCount; }

    Q_INVOKABLE QString placeApp(
        const QString& appId,
        const QString& appName,
        const QString& targetDeviceName,
        int positionEnum,
        const QRectF& rect = QRectF(),
        const QString& appType = QStringLiteral("Application")
    );

    Q_INVOKABLE bool updatePlacementRect(const QString& placementId, const QRectF& newRect);
    Q_INVOKABLE QRectF placementRect(const QString& placementId) const;
    Q_INVOKABLE bool removePlacement(const QString& placementId);
    Q_INVOKABLE void clearAll();

    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE QVariantMap getById(const QString& placementId) const;
    Q_INVOKABLE QStringList placedAppIds() const;

    void setMonitorBounds(const QString& deviceName, const QRectF& bounds);
    void setMonitors(const QList<MonitorInfo>& monitors);

    QList<Placement> toPlacements() const;
    void loadPlacements(const QList<Placement>& placements, const QList<AppItem>& catalog = {});

signals:
    void overlapStateChanged();
    void countChanged();
    void placementsChanged();

private:
    void recomputeOverlaps();
    QRectF clampToConstraints(const QString& deviceName, const QRectF& rect) const;

    QList<PlacementItem> m_placements;
    QHash<QString, QRectF> m_monitorBounds;
    QList<MonitorInfo> m_monitors;
    bool m_hasOverlaps = false;
    int m_invalidCount = 0;
};

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariantMap>

#include "../MonitorInfo.h"
#include "../../services/MonitorService.h"

class MonitorCanvasModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY monitorsChanged)

public:
    enum Roles {
        DeviceNameRole = Qt::UserRole + 1,
        ModelNameRole,
        RectXRole,
        RectYRole,
        RectWidthRole,
        RectHeightRole,
        AvailXRole,
        AvailYRole,
        AvailWidthRole,
        AvailHeightRole,
        IsPrimaryRole,
        ScaleFactorRole
    };
    Q_ENUM(Roles)

    explicit MonitorCanvasModel(MonitorService* service = nullptr, QObject* parent = nullptr);
    ~MonitorCanvasModel() override = default;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_monitors.size(); }

    void setMonitorService(MonitorService* service);
    MonitorService* monitorService() const { return m_monitorService; }

    void setMonitors(const QList<MonitorInfo>& monitors);
    QList<MonitorInfo> monitors() const { return m_monitors; }

    Q_INVOKABLE void refreshMonitors();
    Q_INVOKABLE QVariantMap getMonitor(int index) const;
    Q_INVOKABLE QVariantMap getMonitorByName(const QString& deviceName) const;

signals:
    void monitorsChanged();

private:
    qreal computeScaleFactor(const MonitorInfo& monitor) const;

    QList<MonitorInfo> m_monitors;
    MonitorService* m_monitorService = nullptr;
};

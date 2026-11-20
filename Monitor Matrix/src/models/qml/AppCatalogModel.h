#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "../AppItem.h"
#include "../../services/AppCatalogService.h"

class AppCatalogModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        TargetRole,
        IsPlacedRole,
        IconSourceRole
    };
    Q_ENUM(Roles)

    explicit AppCatalogModel(QObject* parent = nullptr);
    ~AppCatalogModel() override = default;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_filteredApps.size(); }
    QString searchFilter() const { return m_filterText; }

    void setConfigFile(const QString& configFile);
    QString configFile() const;
    void load();

    void setApps(const QList<AppItem>& apps);
    QList<AppItem> apps() const { return m_allApps; }

    Q_INVOKABLE bool addApplication(const QString& filePath, const QString& name);
    Q_INVOKABLE bool addUrl(const QString& url, const QString& name);
    Q_INVOKABLE bool removeApplication(const QString& appId);
    Q_INVOKABLE void setSearchFilter(const QString& filterText);

    Q_INVOKABLE bool isPlaced(const QString& appId) const;
    Q_INVOKABLE void setPlacedAppIds(const QStringList& appIds);
    void updatePlacedApps(const QSet<QString>& placedIds);

    Q_INVOKABLE QVariantMap get(int index) const;

signals:
    void countChanged();
    void searchFilterChanged();
    void catalogChanged();

private:
    void applyFilter();

    QList<AppItem> m_allApps;
    QList<AppItem> m_filteredApps;
    QSet<QString> m_placedAppIds;
    QString m_filterText;
    AppCatalogService m_catalogService;
};

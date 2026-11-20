#include "AppCatalogModel.h"

#include <algorithm>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include "../../managers/ConfigManager.h"
#include "../../platform/windows/WindowsShortcutResolver.h"

AppCatalogModel::AppCatalogModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int AppCatalogModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_filteredApps.size();
}

QVariant AppCatalogModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filteredApps.size()) {
        return QVariant();
    }

    const AppItem& item = m_filteredApps.at(index.row());

    switch (role) {
    case IdRole:
        return item.id;
    case NameRole:
        return item.name;
    case TypeRole:
        return item.type;
    case TargetRole:
        return item.path;
    case IsPlacedRole:
        return m_placedAppIds.contains(item.id);
    case IconSourceRole:
        if (item.type.compare(QStringLiteral("URL"), Qt::CaseInsensitive) == 0) {
            return QStringLiteral("image://appicon/url");
        }
        return item.path.isEmpty() ? QStringLiteral("image://appicon/app") : item.path;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AppCatalogModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[TypeRole] = "type";
    roles[TargetRole] = "target";
    roles[IsPlacedRole] = "isPlaced";
    roles[IconSourceRole] = "iconSource";
    return roles;
}

void AppCatalogModel::setConfigFile(const QString& configFile)
{
    m_catalogService.setConfigFile(configFile);
}

QString AppCatalogModel::configFile() const
{
    return m_catalogService.configFile();
}

void AppCatalogModel::load()
{
    setApps(m_catalogService.loadApplications());
}

void AppCatalogModel::setApps(const QList<AppItem>& apps)
{
    beginResetModel();
    m_allApps = apps;
    applyFilter();
    endResetModel();
    emit countChanged();
    emit catalogChanged();
}

bool AppCatalogModel::addApplication(const QString& filePath, const QString& name)
{
    const QString trimmedPath = filePath.trimmed();
    const QString trimmedName = name.trimmed();

    if (trimmedPath.isEmpty() || trimmedName.isEmpty()) {
        return false;
    }

    const QFileInfo fileInfo(trimmedPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    const QString suffix = fileInfo.suffix();
    if (suffix.compare(QStringLiteral("exe"), Qt::CaseInsensitive) != 0 &&
        suffix.compare(QStringLiteral("lnk"), Qt::CaseInsensitive) != 0) {
        return false;
    }

    // Resolve shortcut if needed
    QString targetExecutable = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    if (suffix.compare(QStringLiteral("lnk"), Qt::CaseInsensitive) == 0) {
        QString resolvedPath;
        QString resolveError;
        if (!WindowsShortcutResolver::resolveExecutableTarget(fileInfo.absoluteFilePath(), &resolvedPath, &resolveError)) {
            return false;
        }
        targetExecutable = resolvedPath;
    }

    // Check duplicate
    for (const AppItem& existing : m_allApps) {
        if (QDir::toNativeSeparators(existing.path).compare(targetExecutable, Qt::CaseInsensitive) == 0) {
            return false;
        }
    }

    if (!m_catalogService.configFile().isEmpty()) {
        const AppCatalogResult result = m_catalogService.addApplicationFile(m_allApps, filePath, trimmedName);
        if (!result.success) {
            return false;
        }
        beginResetModel();
        m_allApps = result.apps;
        applyFilter();
        endResetModel();
    } else {
        beginResetModel();
        AppItem newItem = ConfigManager::createApplicationFromExecutable(targetExecutable, trimmedName, m_allApps);
        m_allApps.append(newItem);
        applyFilter();
        endResetModel();
    }

    emit countChanged();
    emit catalogChanged();
    return true;
}

bool AppCatalogModel::addUrl(const QString& url, const QString& name)
{
    const QString trimmedUrl = url.trimmed();
    const QString trimmedName = name.trimmed();

    if (trimmedUrl.isEmpty() || trimmedName.isEmpty()) {
        return false;
    }

    const QUrl parsedUrl = QUrl::fromUserInput(trimmedUrl);
    if (!parsedUrl.isValid() || parsedUrl.scheme().isEmpty() ||
        (parsedUrl.scheme().compare(QStringLiteral("http"), Qt::CaseInsensitive) != 0 &&
         parsedUrl.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0) ||
        parsedUrl.host().isEmpty()) {
        return false;
    }

    const QString host = parsedUrl.host();
    if (!host.contains(QLatin1Char('.')) && host.compare(QStringLiteral("localhost"), Qt::CaseInsensitive) != 0) {
        return false;
    }

    const QString normalizedUrl = parsedUrl.toString();

    // Check duplicate
    for (const AppItem& existing : m_allApps) {
        if (existing.type.compare(QStringLiteral("URL"), Qt::CaseInsensitive) == 0 &&
            existing.path.compare(normalizedUrl, Qt::CaseInsensitive) == 0) {
            return false;
        }
    }

    if (!m_catalogService.configFile().isEmpty()) {
        const AppCatalogResult result = m_catalogService.addUrlLink(m_allApps, normalizedUrl, trimmedName);
        if (!result.success) {
            return false;
        }
        beginResetModel();
        m_allApps = result.apps;
        applyFilter();
        endResetModel();
    } else {
        beginResetModel();
        AppItem newItem = ConfigManager::createUrlLink(normalizedUrl, trimmedName, m_allApps);
        m_allApps.append(newItem);
        applyFilter();
        endResetModel();
    }

    emit countChanged();
    emit catalogChanged();
    return true;
}

bool AppCatalogModel::removeApplication(const QString& appId)
{
    auto it = std::find_if(m_allApps.begin(), m_allApps.end(), [&](const AppItem& item) {
        return item.id == appId;
    });

    if (it == m_allApps.end()) {
        return false;
    }

    if (!m_catalogService.configFile().isEmpty()) {
        const AppCatalogResult result = m_catalogService.removeApplication(m_allApps, appId);
        if (!result.success) {
            return false;
        }
        beginResetModel();
        m_allApps = result.apps;
        applyFilter();
        endResetModel();
    } else {
        beginResetModel();
        m_allApps.erase(it);
        applyFilter();
        endResetModel();
    }

    if (m_placedAppIds.contains(appId)) {
        m_placedAppIds.remove(appId);
    }

    emit countChanged();
    emit catalogChanged();
    return true;
}

void AppCatalogModel::setSearchFilter(const QString& filterText)
{
    if (m_filterText == filterText) {
        return;
    }

    beginResetModel();
    m_filterText = filterText;
    applyFilter();
    endResetModel();

    emit searchFilterChanged();
    emit countChanged();
}

bool AppCatalogModel::isPlaced(const QString& appId) const
{
    return m_placedAppIds.contains(appId);
}

void AppCatalogModel::setPlacedAppIds(const QStringList& appIds)
{
    QSet<QString> set;
    for (const QString& id : appIds) {
        set.insert(id);
    }
    updatePlacedApps(set);
}

void AppCatalogModel::updatePlacedApps(const QSet<QString>& placedIds)
{
    if (m_placedAppIds == placedIds) {
        return;
    }

    m_placedAppIds = placedIds;
    if (!m_filteredApps.isEmpty()) {
        emit dataChanged(index(0, 0), index(m_filteredApps.size() - 1, 0), { IsPlacedRole });
    }
}

QVariantMap AppCatalogModel::get(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_filteredApps.size()) {
        return map;
    }

    const AppItem& item = m_filteredApps.at(index);
    map[QStringLiteral("id")] = item.id;
    map[QStringLiteral("name")] = item.name;
    map[QStringLiteral("type")] = item.type;
    map[QStringLiteral("target")] = item.path;
    map[QStringLiteral("isPlaced")] = m_placedAppIds.contains(item.id);
    map[QStringLiteral("iconSource")] = (item.type.compare(QStringLiteral("URL"), Qt::CaseInsensitive) == 0)
        ? QStringLiteral("image://appicon/url")
        : item.path;
    return map;
}

void AppCatalogModel::applyFilter()
{
    const QString filter = m_filterText.trimmed();
    if (filter.isEmpty()) {
        m_filteredApps = m_allApps;
        return;
    }

    m_filteredApps.clear();
    for (const AppItem& item : m_allApps) {
        if (item.name.contains(filter, Qt::CaseInsensitive) ||
            item.type.contains(filter, Qt::CaseInsensitive) ||
            item.path.contains(filter, Qt::CaseInsensitive)) {
            m_filteredApps.append(item);
        }
    }
}

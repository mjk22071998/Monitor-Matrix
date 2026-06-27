#include "ConfigManager.h"
#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qdir.h>
#include <memory>
#include "../strategies/ExecutableStrategy.h"
#include "../strategies/URLStrategy.h"

#include <qfileiconprovider.h>
#include <qfileinfo.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qregularexpression.h>
#include <qsavefile.h>
#include <qset.h>
#include <qurl.h>

static QString normalizedType(const QString& type, const QString& path)
{
	QString ltype = type.toLower();

	if (ltype == "application" || ltype == "app")
		return QStringLiteral("Application");

	if (ltype == "url" || ltype == "web" || ltype == "website")
		return QStringLiteral("URL");

	if (path.startsWith("http://", Qt::CaseInsensitive) ||
		path.startsWith("https://", Qt::CaseInsensitive)) {
		return QStringLiteral("URL");
	}

	return QStringLiteral("Application");
}

static QString makeUniqueId(const QString& name, const QList<AppItem>& existingApps)
{
	QString base = name.toLower().trimmed();
	base.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
	base.replace(QRegularExpression(QStringLiteral("^-+|-+$")), QString());

	if (base.isEmpty())
		base = QStringLiteral("app");

	QSet<QString> ids;
	for (const AppItem& app : existingApps)
		ids.insert(app.id.toLower());

	QString candidate = base;
	int suffix = 2;

	while (ids.contains(candidate.toLower())) {
		candidate = QStringLiteral("%1-%2").arg(base).arg(suffix);
		++suffix;
	}

	return candidate;
}

static void attachLaunchStrategy(AppItem& item)
{
	const QString type = normalizedType(item.type, item.path);
	item.type = type;

	if (type == QStringLiteral("Application")) {
		item.launchStrategy = std::make_shared<ExecutableStrategy>(item.path);
	}
	else {
		item.launchStrategy = std::make_shared<URLStrategy>(item.path);
	}
}

static QIcon loadIcon(const QString &type, const QString &pathOrURL)
{
	if (type.toLower() == "url") {
		return QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_ComputerIcon);
	}

	QFileIconProvider provider;
	QFileInfo info(pathOrURL);
	if (info.exists()) {
		return provider.icon(info);
	}

	return QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_FileIcon);
}

QList<AppItem> ConfigManager::getApplications(const QString& fileName)
{
	QList<AppItem> apps;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << "Cannot open config file:" << fileName;
		return apps;
	}
	QByteArray bytes = file.readAll();
	QJsonParseError parseError;
	QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
	if (parseError.error != QJsonParseError::NoError) {
		qWarning() << "JSON parse error in" << fileName << ":" << parseError.errorString();
		return apps;
	}

	QJsonArray array;
	if (document.isArray()) {
		array = document.array();
	}
	else if (document.isObject()) {
		QJsonObject root = document.object();
		if (root.contains("apps") && root["apps"].isArray()) {
			array = root["apps"].toArray();
		}
		else {
			qWarning() << "Invalid config format: expected array or object with 'apps' array";
			return apps;
		}
	}
	else {
		qWarning() << "Invalid JSON root in config file:" << fileName;
		return apps;
	}

	for (const auto& val : array) {
		if (!val.isObject()) {
			qWarning() << "Skipping non-object entry in apps array";
			continue;
		}
		QJsonObject object = val.toObject();
		AppItem item;
		QString id = object["id"].toString().trimmed();
		QString name = object["name"].toString().trimmed();
		QString type = object["type"].toString().trimmed();
		QString path = object["path"].toString().trimmed();

		if (name.isEmpty() || path.isEmpty()) {
			qWarning() << "Skipping item with missing name or path:" << object;
			continue;
		}

		item.id = id;
		item.name = name;
		item.type = type;
		item.path = path;
		item.icon = loadIcon(item.type, item.path);

		attachLaunchStrategy(item);

		apps.append(item);
	}
	return apps;
}

bool ConfigManager::saveApplications(const QString& fileName, const QList<AppItem>& apps)
{
	QFileInfo fileInfo(fileName);
	QDir dir(fileInfo.absolutePath());

	if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
		qWarning() << "Cannot create config directory:" << fileInfo.absolutePath();
		return false;
	}

	QJsonArray array;

	for (const AppItem& app : apps) {
		QJsonObject object;
		object.insert(QStringLiteral("id"), app.id);
		object.insert(QStringLiteral("name"), app.name);
		object.insert(QStringLiteral("type"), normalizedType(app.type, app.path));
		object.insert(QStringLiteral("path"), app.path);
		array.append(object);
	}

	QSaveFile file(fileName);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning() << "Cannot open app config for writing:" << fileName;
		return false;
	}

	file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));

	if (!file.commit()) {
		qWarning() << "Cannot commit app config:" << fileName;
		return false;
	}

	return true;
}

AppItem ConfigManager::createApplicationFromExecutable(
	const QString& executablePath,
	const QString& displayName,
	const QList<AppItem>& existingApps)
{
	QFileInfo fileInfo(executablePath);
	const QString name = displayName.trimmed().isEmpty()
		? fileInfo.completeBaseName()
		: displayName.trimmed();

	AppItem item;
	item.id = makeUniqueId(name, existingApps);
	item.name = name;
	item.type = QStringLiteral("Application");
	item.path = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
	item.icon = loadIcon(item.type, item.path);
	attachLaunchStrategy(item);

	return item;
}

AppItem ConfigManager::createUrlLink(
	const QString& url,
	const QString& displayName,
	const QList<AppItem>& existingApps)
{
	QUrl parsedUrl = QUrl::fromUserInput(url.trimmed());
	const QString normalizedUrl = parsedUrl.toString();
	QString name = displayName.trimmed();

	if (name.isEmpty()) {
		name = parsedUrl.host();
	}

	if (name.isEmpty()) {
		name = QStringLiteral("Link");
	}

	AppItem item;
	item.id = makeUniqueId(name, existingApps);
	item.name = name;
	item.type = QStringLiteral("URL");
	item.path = normalizedUrl;
	item.icon = loadIcon(item.type, item.path);
	attachLaunchStrategy(item);

	return item;
}

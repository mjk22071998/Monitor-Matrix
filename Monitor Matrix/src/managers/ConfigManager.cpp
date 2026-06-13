#include "ConfigManager.h"
#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <memory>
#include "../strategies/ExecutableStrategy.h"
#include "../strategies/URLStrategy.h"

#include <qfileiconprovider.h>
#include <qfileinfo.h>
#include <qapplication.h>
#include <qstyle.h>

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

		QString ltype = type.toLower();
		bool isApplication = false;
		bool isUrl = false;
		if (ltype == "application" || ltype == "app") {
			isApplication = true;
		}
		else if (ltype == "url" || ltype == "web" || ltype == "website") {
			isUrl = true;
		}
		else {
			// Infer from path
			if (path.startsWith("http://", Qt::CaseInsensitive) || path.startsWith("https://", Qt::CaseInsensitive)) {
				isUrl = true;
			}
			else {
				isApplication = true;
			}
		}

		if (isApplication) {
			item.launchStrategy = std::make_shared<ExecutableStrategy>(item.path);
		}
		else {
			item.launchStrategy = std::make_shared<URLStrategy>(item.path);
		}

		apps.append(item);
	}
	return apps;
}

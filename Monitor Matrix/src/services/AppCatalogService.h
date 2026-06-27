#pragma once

#include <QList>
#include <QString>

#include "../models/AppItem.h"

enum class AppCatalogMessageType {
	None,
	Information,
	Warning
};

struct AppCatalogResult {
	bool success = false;
	QList<AppItem> apps;
	AppCatalogMessageType messageType = AppCatalogMessageType::None;
	QString title;
	QString message;
	QString statusMessage;
	QString appName;
};

class AppCatalogService {
public:
	void setConfigFile(const QString& configFile);
	QString configFile() const;

	QList<AppItem> loadApplications() const;
	AppCatalogResult addApplicationFile(
		const QList<AppItem>& apps,
		const QString& filePath,
		const QString& displayName) const;
	AppCatalogResult addUrlLink(
		const QList<AppItem>& apps,
		const QString& url,
		const QString& displayName) const;
	AppCatalogResult removeApplication(
		const QList<AppItem>& apps,
		const QString& appId) const;

private:
	QString m_configFile;

	AppCatalogResult addExecutableApp(
		const QList<AppItem>& apps,
		const QString& executablePath,
		const QString& displayName) const;
	AppCatalogResult saveUpdatedApplications(
		const QList<AppItem>& apps,
		const QString& statusMessage = QString(),
		const QString& appName = QString()) const;
	AppCatalogResult messageResult(
		AppCatalogMessageType type,
		const QString& title,
		const QString& message) const;
};

#include "AppCatalogService.h"

#include <algorithm>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include "../managers/ConfigManager.h"
#include "../platform/windows/WindowsShortcutResolver.h"

void AppCatalogService::setConfigFile(const QString& configFile)
{
	m_configFile = configFile;
}

QString AppCatalogService::configFile() const
{
	return m_configFile;
}

QList<AppItem> AppCatalogService::loadApplications() const
{
	return ConfigManager::getApplications(m_configFile);
}

AppCatalogResult AppCatalogService::messageResult(
	AppCatalogMessageType type,
	const QString& title,
	const QString& message) const
{
	AppCatalogResult result;
	result.messageType = type;
	result.title = title;
	result.message = message;
	return result;
}

AppCatalogResult AppCatalogService::saveUpdatedApplications(
	const QList<AppItem>& apps,
	const QString& statusMessage,
	const QString& appName) const
{
	if (!ConfigManager::saveApplications(m_configFile, apps)) {
		return messageResult(
			AppCatalogMessageType::Warning,
			QStringLiteral("Apps Not Saved"),
			QStringLiteral("Cannot save application list:\n%1").arg(m_configFile)
		);
	}

	AppCatalogResult result;
	result.success = true;
	result.apps = apps;
	result.statusMessage = statusMessage;
	result.appName = appName;
	return result;
}

AppCatalogResult AppCatalogService::addApplicationFile(
	const QList<AppItem>& apps,
	const QString& filePath,
	const QString& displayName) const
{
	const QFileInfo fileInfo(filePath);

	if (!fileInfo.exists() || !fileInfo.isFile()) {
		return messageResult(
			AppCatalogMessageType::Warning,
			QStringLiteral("Invalid Application"),
			QStringLiteral("Choose a valid Windows executable or shortcut.")
		);
	}

	if (fileInfo.suffix().compare(QStringLiteral("exe"), Qt::CaseInsensitive) == 0) {
		return addExecutableApp(apps, fileInfo.absoluteFilePath(), displayName);
	}

	if (fileInfo.suffix().compare(QStringLiteral("lnk"), Qt::CaseInsensitive) == 0) {
		QString resolvedPath;
		QString resolveError;

		if (!WindowsShortcutResolver::resolveExecutableTarget(
			fileInfo.absoluteFilePath(),
			&resolvedPath,
			&resolveError)) {
			qWarning().noquote()
				<< "Shortcut rejected"
				<< "shortcut="
				<< fileInfo.absoluteFilePath()
				<< "reason="
				<< resolveError;

			return messageResult(
				AppCatalogMessageType::Warning,
				QStringLiteral("Invalid Shortcut"),
				QStringLiteral("%1 does not point to a valid executable.\n\n%2")
					.arg(fileInfo.fileName(), resolveError)
			);
		}

		const QString name = displayName.trimmed().isEmpty()
			? fileInfo.completeBaseName()
			: displayName.trimmed();

		qInfo().noquote()
			<< "Shortcut resolved"
			<< "shortcut="
			<< fileInfo.absoluteFilePath()
			<< "target="
			<< resolvedPath;

		return addExecutableApp(apps, resolvedPath, name);
	}

	return messageResult(
		AppCatalogMessageType::Warning,
		QStringLiteral("Invalid Application"),
		QStringLiteral("Only .exe and .lnk files can be added as applications.")
	);
}

AppCatalogResult AppCatalogService::addExecutableApp(
	const QList<AppItem>& apps,
	const QString& executablePath,
	const QString& displayName) const
{
	const QFileInfo fileInfo(executablePath);

	if (!fileInfo.exists() ||
		!fileInfo.isFile() ||
		fileInfo.suffix().compare(QStringLiteral("exe"), Qt::CaseInsensitive) != 0) {
		return messageResult(
			AppCatalogMessageType::Warning,
			QStringLiteral("Invalid Application"),
			QStringLiteral("Choose a valid Windows executable file.")
		);
	}

	const QString normalizedPath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());

	for (const AppItem& app : apps) {
		if (QDir::toNativeSeparators(app.path)
			.compare(normalizedPath, Qt::CaseInsensitive) == 0) {
			return messageResult(
				AppCatalogMessageType::Information,
				QStringLiteral("Application Already Added"),
				QStringLiteral("%1 is already in the app list.").arg(app.name)
			);
		}
	}

	QList<AppItem> updatedApps = apps;
	updatedApps.append(
		ConfigManager::createApplicationFromExecutable(
			normalizedPath,
			displayName,
			updatedApps
		)
	);

	AppCatalogResult result = saveUpdatedApplications(
		updatedApps,
		QStringLiteral("Application added"),
		updatedApps.last().name
	);

	if (result.success) {
		qInfo().noquote()
			<< "Application added"
			<< "path="
			<< normalizedPath;
	}

	return result;
}

AppCatalogResult AppCatalogService::addUrlLink(
	const QList<AppItem>& apps,
	const QString& url,
	const QString& displayName) const
{
	const QUrl parsedUrl = QUrl::fromUserInput(url.trimmed());

	if (!parsedUrl.isValid() ||
		parsedUrl.scheme().isEmpty() ||
		(parsedUrl.scheme().compare(QStringLiteral("http"), Qt::CaseInsensitive) != 0 &&
			parsedUrl.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) != 0)) {
		return messageResult(
			AppCatalogMessageType::Warning,
			QStringLiteral("Invalid URL"),
			QStringLiteral("Enter a valid http or https link.")
		);
	}

	const QString normalizedUrl = parsedUrl.toString();

	for (const AppItem& app : apps) {
		if (app.type.compare(QStringLiteral("URL"), Qt::CaseInsensitive) == 0 &&
			app.path.compare(normalizedUrl, Qt::CaseInsensitive) == 0) {
			return messageResult(
				AppCatalogMessageType::Information,
				QStringLiteral("Link Already Added"),
				QStringLiteral("%1 is already in the app list.").arg(app.name)
			);
		}
	}

	QList<AppItem> updatedApps = apps;
	updatedApps.append(
		ConfigManager::createUrlLink(
			normalizedUrl,
			displayName,
			updatedApps
		)
	);

	AppCatalogResult result = saveUpdatedApplications(
		updatedApps,
		QStringLiteral("URL link added"),
		updatedApps.last().name
	);

	if (result.success) {
		qInfo().noquote()
			<< "URL link added"
			<< "url="
			<< normalizedUrl;
	}

	return result;
}

AppCatalogResult AppCatalogService::removeApplication(
	const QList<AppItem>& apps,
	const QString& appId) const
{
	auto it = std::find_if(apps.begin(), apps.end(), [&](const AppItem& app) {
		return app.id == appId;
	});

	if (it == apps.end()) {
		return messageResult(
			AppCatalogMessageType::Warning,
			QStringLiteral("Application Not Found"),
			QStringLiteral("The selected application is no longer in the app list.")
		);
	}

	const QString appName = it->name;
	QList<AppItem> updatedApps = apps;
	updatedApps.erase(
		std::remove_if(updatedApps.begin(), updatedApps.end(), [&](const AppItem& app) {
			return app.id == appId;
		}),
		updatedApps.end()
	);

	AppCatalogResult result = saveUpdatedApplications(
		updatedApps,
		QStringLiteral("Application removed"),
		appName
	);

	if (result.success) {
		qInfo().noquote()
			<< "Application removed"
			<< "appId="
			<< appId
			<< "name="
			<< appName;
	}

	return result;
}

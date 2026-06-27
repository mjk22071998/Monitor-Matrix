#include "MainWindow.h"

#include <algorithm>

#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qset.h>
#include <qstatusbar.h>

#include "../dialog/AppEditDialog.h"
#include "../dialog/AuthDialog.h"

namespace {

void showCatalogMessage(QWidget* parent, const AppCatalogResult& result)
{
	if (result.messageType == AppCatalogMessageType::Information) {
		QMessageBox::information(parent, result.title, result.message);
	}
	else if (result.messageType == AppCatalogMessageType::Warning) {
		QMessageBox::warning(parent, result.title, result.message);
	}
}

}

void MainWindow::updatePlacedApps()
{
	if (!p_appListView) {
		return;
	}

	QSet<QString> placedAppIds;

	for (const Placement& placement : m_placements) {
		placedAppIds.insert(placement.appId);
	}

	p_appListView->setPlacedApps(placedAppIds);
}

bool MainWindow::addApplicationFile(const QString& filePath, const QString& displayName)
{
	const AppCatalogResult result =
		m_appCatalog.addApplicationFile(m_apps, filePath, displayName);

	if (!result.success) {
		showCatalogMessage(this, result);
		return false;
	}

	m_apps = result.apps;
	p_appListView->setApps(m_apps);
	updatePlacedApps();
	return true;
}

bool MainWindow::addUrlLink(const QString& url, const QString& displayName)
{
	const AppCatalogResult result =
		m_appCatalog.addUrlLink(m_apps, url, displayName);

	if (!result.success) {
		showCatalogMessage(this, result);
		return false;
	}

	m_apps = result.apps;
	p_appListView->setApps(m_apps);
	updatePlacedApps();
	return true;
}

void MainWindow::openAddAppDialog()
{
	if (!AuthDialog::authenticate(this, QStringLiteral("app management"))) {
		return;
	}

	AppEditDialog dialog(this);

	if (dialog.exec() != QDialog::Accepted) {
		return;
	}

	if (dialog.appType() == QStringLiteral("URL")) {
		addUrlLink(dialog.target(), dialog.appName());
	}
	else {
		addApplicationFile(dialog.target(), dialog.appName());
	}
}

void MainWindow::addDroppedApplicationFiles(const QStringList& filePaths)
{
	if (filePaths.isEmpty()) {
		return;
	}

	if (!AuthDialog::authenticate(this, QStringLiteral("app management"))) {
		return;
	}

	int addedCount = 0;

	for (const QString& path : filePaths) {
		const QFileInfo fileInfo(path);
		if (addApplicationFile(path, fileInfo.completeBaseName())) {
			++addedCount;
		}
	}

	if (addedCount > 0) {
		statusBar()->showMessage(
			QStringLiteral("%1 application%2 added")
				.arg(addedCount)
				.arg(addedCount == 1 ? QString() : QStringLiteral("s")),
			4000
		);
	}
}

void MainWindow::removeApp(const QString& appId)
{
	if (appId.isEmpty()) {
		return;
	}

	if (!AuthDialog::authenticate(this, QStringLiteral("app management"))) {
		return;
	}

	auto it = std::find_if(m_apps.begin(), m_apps.end(), [&](const AppItem& app) {
		return app.id == appId;
	});

	if (it == m_apps.end()) {
		return;
	}

	const QString appName = it->name;
	const int placementCount = std::count_if(
		m_placements.begin(),
		m_placements.end(),
		[&](const Placement& placement) {
			return placement.appId == appId;
		}
	);

	QString message = QStringLiteral("Remove %1 from the app list?").arg(appName);
	if (placementCount > 0) {
		message += QStringLiteral("\n\nThis will also remove %1 placement%2 from the current layout.")
			.arg(placementCount)
			.arg(placementCount == 1 ? QString() : QStringLiteral("s"));
	}

	const QMessageBox::StandardButton answer = QMessageBox::question(
		this,
		QStringLiteral("Remove Application"),
		message,
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No
	);

	if (answer != QMessageBox::Yes) {
		return;
	}

	const AppCatalogResult result =
		m_appCatalog.removeApplication(m_apps, appId);

	if (!result.success) {
		showCatalogMessage(this, result);
		return;
	}

	m_apps = result.apps;
	p_monitorScene->removePlacementsForApp(appId);
	p_appListView->setApps(m_apps);
	updatePlacedApps();
}

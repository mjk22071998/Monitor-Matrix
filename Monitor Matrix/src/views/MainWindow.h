#pragma once
#include <qmainwindow.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstringlist.h>
#include "../models/AppItem.h"
#include "../services/MonitorService.h"
#include "../services/AppCatalogService.h"
#include "../services/LayoutLaunchService.h"
#include "../scene/MonitorScene.h"
#include "../models/Placement.h"
#include "../theme/AppTheme.h"
#include "AppListView.h"

class QAction;
class QWidget;

class MainWindow : public QMainWindow {
	Q_OBJECT
private:
	MonitorService* p_monitorService;
	MonitorScene* p_monitorScene;
	AppListView* p_appListView;
	QPushButton* p_runButton;
	QList<Placement> m_placements;
	QList<AppItem> m_apps;
	QString m_currentFile;
	AppCatalogService m_appCatalog;
	LayoutLaunchService m_layoutLauncher;
	QWidget* p_overlapStatusWidget = nullptr;
	QLabel* p_overlapStatusIconLabel = nullptr;
	QLabel* p_overlapWarningLabel = nullptr;
	QAction* p_darkThemeAction = nullptr;
	QAction* p_lightThemeAction = nullptr;

	void createMenus();
	void updateTitle();
	void updateThemeActions();
	void updateOverlapStatus(bool hasOverlaps, int invalidPlacementCount);
	void updatePlacedApps();
	bool addApplicationFile(const QString& filePath, const QString& displayName);
	bool addUrlLink(const QString& url, const QString& displayName);

private slots:
	void onRunClicked();
	void save();
	void saveAs();
	void openLayout();
	void openScreenSettings();
	void openLogsFolder();
	void openAddAppDialog();
	void removeApp(const QString& appId);
	void addDroppedApplicationFiles(const QStringList& filePaths);
	void setTheme(AppThemeMode mode);
public:
	explicit MainWindow(QWidget* parent = nullptr);
};

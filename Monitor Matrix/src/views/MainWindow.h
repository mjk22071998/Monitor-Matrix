#pragma once
#include <qmainwindow.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include "../services/MonitorService.h"
#include "../scene/MonitorScene.h"
#include "../managers/ConfigManager.h"
#include "../models/Placement.h"
#include "AppListView.h"

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
	QLabel* p_overlapWarningLabel = nullptr;

	void createMenus();
	void updateTitle();

private slots:
	void onRunClicked();
	void save();
	void saveAs();
	void openLayout();
public:
	explicit MainWindow(QWidget* parent = nullptr);
};
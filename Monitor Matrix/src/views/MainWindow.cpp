#include "MainWindow.h"
#include <qapplication.h>
#include <qgraphicsview.h>
#include <qpainter.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qdebug.h>
#include <qdir.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qicon.h>
#include <qkeysequence.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qsize.h>
#include <qstatusbar.h>
#include <qactiongroup.h>
#include <qdesktopservices.h>
#include <qfileinfo.h>
#include <qurl.h>
#include "../managers/LayoutManager.h"
#include "../dialog/AuthDialog.h"
#include "../dialog/ScreenConfigDialog.h"
#include "../utils/AppLogger.h"

MainWindow::MainWindow(QWidget* parent)
{
	setWindowTitle("ScreenDrop");

	createMenus();
	updateTitle();

	p_overlapStatusWidget = new QWidget(this);
	auto* overlapStatusLayout = new QHBoxLayout(p_overlapStatusWidget);
	overlapStatusLayout->setContentsMargins(0, 0, 0, 0);
	overlapStatusLayout->setSpacing(6);

	p_overlapStatusIconLabel = new QLabel(p_overlapStatusWidget);
	p_overlapStatusIconLabel->setFixedSize(16, 16);

	p_overlapWarningLabel = new QLabel(p_overlapStatusWidget);
	p_overlapWarningLabel->setText("Layout has overlapping apps");
	p_overlapWarningLabel->setStyleSheet(
		AppTheme::statusLabelStyle(true)
	);

	overlapStatusLayout->addWidget(p_overlapStatusIconLabel);
	overlapStatusLayout->addWidget(p_overlapWarningLabel);
	p_overlapStatusWidget->setVisible(false);

	statusBar()->addPermanentWidget(p_overlapStatusWidget);

	QWidget* leftPanel = new QWidget();
	leftPanel->setObjectName("leftPanel");
	QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
	leftLayout->setContentsMargins(0, 0, 0, 0);
	leftLayout->setSpacing(0);

	
	p_appListView = new AppListView();
	m_appCatalog.setConfigFile(QApplication::applicationDirPath() + "/config/apps.json");
	m_apps = m_appCatalog.loadApplications();
	p_appListView->setApps(m_apps);
	leftLayout->addWidget(p_appListView);

	p_runButton = new QPushButton("Run Layout");
	p_runButton->setObjectName("runButton");
	p_runButton->setIcon(QIcon(QStringLiteral(":/icons/action-run.svg")));
	p_runButton->setIconSize(QSize(16, 16));
	p_runButton->setEnabled(true);
	p_runButton->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	leftLayout->addWidget(p_runButton);
	leftPanel->setLayout(leftLayout);

	p_monitorService = new MonitorService(this);
	p_monitorScene = new MonitorScene(this);
	p_monitorScene->setMonitors(p_monitorService->getCurrentMonitors());
	
	connect(p_monitorScene, &MonitorScene::placementChanged, this, [this]() {
		m_placements = p_monitorScene->placements();
		updatePlacedApps();
		});
	connect(p_monitorService, &MonitorService::monitorsChanged, this, [this]() {p_monitorScene->setMonitors(p_monitorService->getCurrentMonitors()); });
	connect(p_runButton, &QPushButton::clicked, this, &MainWindow::onRunClicked);
	connect(p_appListView, &AppListView::addRequested,
		this, &MainWindow::openAddAppDialog);
	connect(p_appListView, &AppListView::removeRequested,
		this, &MainWindow::removeApp);
	connect(p_appListView, &AppListView::applicationFilesDropped,
		this, &MainWindow::addDroppedApplicationFiles);
	connect(p_monitorScene, &MonitorScene::overlapStateChanged,
		this, [this](bool hasOverlaps, int invalidPlacementCount) {
			updateOverlapStatus(hasOverlaps, invalidPlacementCount);
		});


	QGraphicsView* graphicsView = new QGraphicsView(p_monitorScene);
	graphicsView->setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::TextAntialiasing);
	graphicsView->setCacheMode(QGraphicsView::CacheNone);
	graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	graphicsView->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
	graphicsView->fitInView(p_monitorScene->sceneRect(), Qt::AspectRatioMode::KeepAspectRatio);

	QSplitter* splitter = new QSplitter(Qt::Orientation::Horizontal);
	splitter->addWidget(leftPanel);
	splitter->addWidget(graphicsView);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);

	setCentralWidget(splitter);
	resize(1366, 768);
	updatePlacedApps();
	
}

void MainWindow::createMenus()
{
	QMenu* menu = menuBar()->addMenu("&File");
	menu->addAction("&Open...", this, &MainWindow::openLayout, QKeySequence::Open);
	menu->addAction("&Save", this, &MainWindow::save, QKeySequence::Save);
	menu->addAction("Save &As", this, &MainWindow::saveAs, QKeySequence::fromString("Ctrl+Shift+S"));

	QMenu* settingsMenu = menuBar()->addMenu("&Settings");
	settingsMenu->addAction("&Screen Configuration...", this, &MainWindow::openScreenSettings);
	settingsMenu->addAction("Open &Logs Folder", this, &MainWindow::openLogsFolder);
	settingsMenu->addSeparator();

	QMenu* themeMenu = settingsMenu->addMenu("&Theme");
	auto* themeGroup = new QActionGroup(this);
	themeGroup->setExclusive(true);

	p_darkThemeAction = themeMenu->addAction("&Dark");
	p_darkThemeAction->setCheckable(true);
	themeGroup->addAction(p_darkThemeAction);

	p_lightThemeAction = themeMenu->addAction("&Light");
	p_lightThemeAction->setCheckable(true);
	themeGroup->addAction(p_lightThemeAction);

	connect(p_darkThemeAction, &QAction::triggered, this, [this]() {
		setTheme(AppThemeMode::Dark);
	});

	connect(p_lightThemeAction, &QAction::triggered, this, [this]() {
		setTheme(AppThemeMode::Light);
	});

	updateThemeActions();
}

void MainWindow::updateTitle()
{
	QString title = "Screen Drop";
	if (!m_currentFile.isEmpty()) {
		QFileInfo fileInfo(m_currentFile);
		title += " - " + fileInfo.fileName();
	}

	setWindowTitle(title);
}

void MainWindow::onRunClicked() {
	m_layoutLauncher.launchPlacements(m_apps, m_placements);
}

void MainWindow::save()
{
	if (m_currentFile.isEmpty()) {
		saveAs();
		return;
	}

	QList<Placement> placements = p_monitorScene->placements();
	QList<MonitorInfo> monitors = p_monitorService->getCurrentMonitors();

	if (LayoutManager::saveToFile(m_currentFile, placements, monitors)) {
		QMessageBox::information(this, "File saved", m_currentFile + " successfully saved");
	}
	else {
		QMessageBox::warning(this, "Error", "Cannot save file " + m_currentFile);
	}
}

void MainWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(
		this,
		"Save File",
		m_currentFile.isEmpty() ? QDir::homePath() : m_currentFile,
		"Screen Drop Layout (*.sav)"
	);

	if (fileName.isEmpty()) {
		return;
	}

	if (!fileName.endsWith(".sav", Qt::CaseInsensitive)) {
		fileName += ".sav";
	}

	m_currentFile = fileName;
	save();
	updateTitle();
}

void MainWindow::openLayout()
{
	QString fileName = QFileDialog::getOpenFileName(
		this,
		"Open File",
		m_currentFile.isEmpty() ? QDir::homePath() : m_currentFile,
		"Screen Drop Layout (*.sav)"
	);

	if (fileName.isEmpty()) {
		return;
	}
	
	QList<Placement> loadedPlacements;
	QList<MonitorInfo> savedMonitors;



	if (LayoutManager::loadFromFile(fileName, loadedPlacements, savedMonitors)) {
		qDebug() << fileName << " File loaded";
	}
	else {
		QMessageBox::warning(this, "Error", "Cannot load file " + m_currentFile);
		return;
	}

	QList<MonitorInfo> currentMonitors = p_monitorService->getCurrentMonitors();
	
	bool monitorsMatched = savedMonitors.size() == currentMonitors.size();
	if (monitorsMatched) {
		for (int i = 0; i < savedMonitors.size(); i++) {
			if (savedMonitors[i].deviceName != currentMonitors[i].deviceName) {
				monitorsMatched = false;
				break;
			}
		}
	}

	if (!monitorsMatched) {
		QMessageBox::StandardButton reply = QMessageBox::question(this, "Monitor Layout Changed",
			"The saved Monitor layout does not match your current layout.\n"
			"Do you want to load the placements anyway?",
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No
		);
		if (reply == QMessageBox::No)
			return;
	}

	p_monitorScene->clearPlacements();
	QMap<QString, QString> appNames;
	for (const auto& app : m_apps) {
		appNames[app.id] = app.name;
	}
	QList<QString> notPlaced;
	for (const auto& placement : loadedPlacements) {
		QString appName = appNames.value(placement.appId, placement.appId);
		if (appName == placement.appId) {
			notPlaced.append(appName);
		}
		else {
			p_monitorScene->addPlacement(placement, appName);
		}
	}

	m_currentFile = fileName;
	updateTitle();

	if (!notPlaced.isEmpty()) {
		QString notPlacedAppNames;
		for (auto appName : notPlaced) {
			notPlacedAppNames += appName + " ";
		}
		notPlacedAppNames=notPlacedAppNames.trimmed();
		QMessageBox::warning(this, "Apps not placed", "These apps not found in current app list, hence not placed " + notPlacedAppNames);
	}
}

void MainWindow::openScreenSettings()
{
	if (!AuthDialog::authenticate(this, QStringLiteral("screen configuration"))) {
		return;
	}

	ScreenConfigDialog screenDialog(this);

	connect(&screenDialog, &ScreenConfigDialog::displaySettingsSaved, this, [this]() {
		p_monitorService->refreshMonitors();
	});

	screenDialog.exec();
}

void MainWindow::openLogsFolder()
{
	QDesktopServices::openUrl(
		QUrl::fromLocalFile(AppLogger::logDirectory())
	);
}

void MainWindow::setTheme(AppThemeMode mode)
{
	AppTheme::apply(*qApp, mode);
	updateThemeActions();

	if (p_monitorScene) {
		p_monitorScene->refreshTheme();
	}

	if (p_overlapWarningLabel && p_overlapWarningLabel->isVisible()) {
		const bool hasOverlaps = p_overlapWarningLabel->text().contains("invalid");
		p_overlapWarningLabel->setStyleSheet(AppTheme::statusLabelStyle(hasOverlaps));
	}
}

void MainWindow::updateOverlapStatus(bool hasOverlaps, int invalidPlacementCount)
{
	if (!p_overlapStatusWidget || !p_overlapStatusIconLabel || !p_overlapWarningLabel) {
		return;
	}

	const QString iconPath = hasOverlaps
		? QStringLiteral(":/icons/status-warning.svg")
		: QStringLiteral(":/icons/status-check.svg");

	p_overlapStatusIconLabel->setPixmap(QIcon(iconPath).pixmap(16, 16));
	p_overlapStatusIconLabel->setToolTip(
		hasOverlaps
		? QStringLiteral("Layout has overlapping apps")
		: QStringLiteral("Layout is valid")
	);

	if (hasOverlaps) {
		p_overlapWarningLabel->setText(
			QString("%1 invalid placement%2")
			.arg(invalidPlacementCount)
			.arg(invalidPlacementCount == 1 ? "" : "s")
		);
	}
	else {
		p_overlapWarningLabel->setText(QStringLiteral("Layout valid"));
	}

	p_overlapWarningLabel->setStyleSheet(AppTheme::statusLabelStyle(hasOverlaps));
	p_overlapStatusWidget->setVisible(true);
}

void MainWindow::updateThemeActions()
{
	if (p_darkThemeAction) {
		p_darkThemeAction->setChecked(AppTheme::mode() == AppThemeMode::Dark);
	}

	if (p_lightThemeAction) {
		p_lightThemeAction->setChecked(AppTheme::mode() == AppThemeMode::Light);
	}
}

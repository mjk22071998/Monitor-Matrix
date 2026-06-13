#include "MainWindow.h"
#include <qapplication.h>
#include <qlist.h>
#include <qgraphicsview.h>
#include <qpainter.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qset.h>
#include <QtConcurrent>
#include <qmenu.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qkeysequence.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include "../strategies/LaunchStrategy.h"
#include "../managers/LayoutManager.h"
#include "../models/AppItem.h"

MainWindow::MainWindow(QWidget* parent)
{
	setWindowTitle("ScreenDrop");

	createMenus();
	updateTitle();

	p_overlapWarningLabel = new QLabel(this);
	p_overlapWarningLabel->setText("⚠ Layout has overlapping apps");
	p_overlapWarningLabel->setStyleSheet(
		"QLabel { color: #ff5050; font-weight: bold; padding: 4px 8px; }"
	);
	p_overlapWarningLabel->setVisible(false);

	statusBar()->addPermanentWidget(p_overlapWarningLabel);

	QWidget* leftPanel = new QWidget();
	leftPanel->setObjectName("leftPanel");
	QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

	
	p_appListView = new AppListView();
	m_apps = ConfigManager::getApplications(QApplication::applicationDirPath() + "/config/apps.json");
	p_appListView->setApps(m_apps);
	leftLayout->addWidget(p_appListView);

	p_runButton = new QPushButton("RUN");
	p_runButton->setObjectName("runButton");
	p_runButton->setEnabled(true);
	p_runButton->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	leftLayout->addWidget(p_runButton);
	leftPanel->setLayout(leftLayout);

	p_monitorService = new MonitorService(this);
	p_monitorScene = new MonitorScene(this);
	p_monitorScene->setMonitors(p_monitorService->getCurrentMonitors());
	
	connect(p_monitorScene, &MonitorScene::placementChanged, this, [this]() {
		m_placements = p_monitorScene->placements();
		});
	connect(p_monitorService, &MonitorService::monitorsChanged, this, [this]() {p_monitorScene->setMonitors(p_monitorService->getCurrentMonitors()); });
	connect(p_runButton, &QPushButton::clicked, this, &MainWindow::onRunClicked);
	connect(p_monitorScene, &MonitorScene::overlapStateChanged,
		this, [this](bool hasOverlaps, int invalidPlacementCount) {
			if (hasOverlaps) {
				p_overlapWarningLabel->setText(
					QString("⚠ %1 invalid placement%2")
					.arg(invalidPlacementCount)
					.arg(invalidPlacementCount == 1 ? "" : "s")
				);

				p_overlapWarningLabel->setStyleSheet(
					"QLabel { color: #ff5050; font-weight: bold; padding: 4px 8px; }"
				);
			}
			else {
				p_overlapWarningLabel->setText("✔ Layout valid");

				p_overlapWarningLabel->setStyleSheet(
					"QLabel { color: #32d2b4; font-weight: bold; padding: 4px 8px; }"
				);
			}

			p_overlapWarningLabel->setVisible(true);
		});


	QGraphicsView* graphicsView = new QGraphicsView(p_monitorScene);
	graphicsView->setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::TextAntialiasing);
	graphicsView->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
	graphicsView->fitInView(p_monitorScene->sceneRect(), Qt::AspectRatioMode::KeepAspectRatio);

	QSplitter* splitter = new QSplitter(Qt::Orientation::Horizontal);
	splitter->addWidget(leftPanel);
	splitter->addWidget(graphicsView);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);

	setCentralWidget(splitter);
	resize(1366, 768);
	
}

void MainWindow::createMenus()
{
	QMenu* menu = menuBar()->addMenu("&File");
	menu->addAction("&Open...", this, &MainWindow::openLayout, QKeySequence::Open);
	menu->addAction("&Save", this, &MainWindow::save, QKeySequence::Save);
	menu->addAction("Save &As", this, &MainWindow::saveAs, QKeySequence::fromString("Ctrl+Shift+S"));
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
	for (const Placement& placement : m_placements) {
		auto it = std::find_if(m_apps.begin(), m_apps.end(), [&](const AppItem & a) {return a.id == placement.appId; });

		if (it != m_apps.end() && it->launchStrategy) {
			auto launchStrategy = it->launchStrategy;
			MonitorInfo monitor = placement.monitor;
			AppPosition position = placement.position;
			if (placement.customRect) {
				QRect z = placement.zoneRect;
				QtConcurrent::run([launchStrategy, monitor, position, z]() {
					launchStrategy->launch((MonitorInfo&)monitor, position, z);
				});
			}
			else {
				QtConcurrent::run([launchStrategy, monitor, position]() {
					launchStrategy->launch((MonitorInfo&)monitor, position);
				});
			}
		}
	}
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

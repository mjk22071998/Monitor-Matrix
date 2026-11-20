#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include "../controllers/ScreenDropEngine.h"
#include "../controllers/DisplayConfigBridge.h"
#include "../models/qml/AppCatalogModel.h"
#include "../models/qml/MonitorCanvasModel.h"
#include "../models/qml/PlacementModel.h"

class EngineTests : public QObject {
    Q_OBJECT

private slots:
    // Review Focus 1: Unsaved changes dirty state
    void testDirtyStateTracking() {
        ScreenDropEngine engine;
        QVERIFY(!engine.isDirty());
        engine.markDirty();
        QVERIFY(engine.isDirty());
        engine.markClean();
        QVERIFY(!engine.isDirty());
    }

    // Review Focus 4: Monitor mismatch detection
    void testMonitorMismatchSignal() {
        ScreenDropEngine engine;
        QSignalSpy spy(&engine, &ScreenDropEngine::monitorMismatchDetected);
        engine.simulateMismatchLoad();
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toInt() != arguments.at(1).toInt());
    }

    void testThemeToggle() {
        ScreenDropEngine engine;
        QSignalSpy spy(&engine, &ScreenDropEngine::darkThemeChanged);
        bool initial = engine.darkTheme();
        engine.setDarkTheme(!initial);
        QCOMPARE(engine.darkTheme(), !initial);
        QCOMPARE(spy.count(), 1);
        engine.setDarkTheme(initial);
        QCOMPARE(engine.darkTheme(), initial);
        QCOMPARE(spy.count(), 2);
    }

    void testPlacementModelWiringAndDirtyState() {
        ScreenDropEngine engine;
        PlacementModel placements;
        AppCatalogModel catalog;
        MonitorCanvasModel monitors;

        engine.setPlacementModel(&placements);
        engine.setCatalogModel(&catalog);
        engine.setCanvasModel(&monitors);

        QVERIFY(!engine.isDirty());

        // Placing an app should mark engine dirty
        QString pId = placements.placeApp("app1", "App One", "\\.\\DISPLAY1", 0, QRectF(0, 0, 100, 100));
        QVERIFY(!pId.isEmpty());
        QVERIFY(engine.isDirty());

        engine.markClean();
        QVERIFY(!engine.isDirty());

        // Removing placement should also mark engine dirty
        QVERIFY(placements.removePlacement(pId));
        QVERIFY(engine.isDirty());
    }

    void testSaveAndLoadLayout() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QString layoutPath = tempDir.filePath("test_layout.sav");

        ScreenDropEngine engine;
        PlacementModel placements;
        AppCatalogModel catalog;
        MonitorCanvasModel monitors;

        engine.setPlacementModel(&placements);
        engine.setCatalogModel(&catalog);
        engine.setCanvasModel(&monitors);

        placements.placeApp("testApp", "Test App", "\\.\\DISPLAY1", 0, QRectF(10, 10, 200, 200));
        QVERIFY(engine.isDirty());

        // Save layout
        QVERIFY(engine.saveLayout(layoutPath));
        QVERIFY(QFile::exists(layoutPath));
        QVERIFY(!engine.isDirty());
        QCOMPARE(engine.currentFileName(), QStringLiteral("test_layout.sav"));

        // Clear placements
        placements.clearAll();
        QCOMPARE(placements.count(), 0);
        engine.markClean();

        // Load layout
        QVERIFY(engine.loadLayout(layoutPath));
        QVERIFY(!engine.isDirty());
        QCOMPARE(placements.count(), 1);
        QCOMPARE(engine.currentFileName(), QStringLiteral("test_layout.sav"));
    }

    void testDisplayConfigBridge() {
        DisplayConfigBridge bridge;
        bridge.refreshDisplays();
        int count = bridge.displayCount();
        QVERIFY(count >= 0);
        QVERIFY(!bridge.hasChanges());

        if (count > 0) {
            QVariantMap display = bridge.getDisplay(0);
            QVERIFY(display.contains("deviceName"));
            QVERIFY(display.contains("displayName"));
            QVERIFY(display.contains("x"));
            QVERIFY(display.contains("y"));
            QVERIFY(display.contains("width"));
            QVERIFY(display.contains("height"));
            QVERIFY(display.contains("isPrimary"));
            QVERIFY(display.contains("resolutions"));

            int origX = display["x"].toInt();
            int origY = display["y"].toInt();

            // Set new position
            QVERIFY(bridge.setDisplayPosition(0, origX + 100, origY + 50));
            QVERIFY(bridge.hasChanges());

            // Revert changes
            bridge.revertChanges();
            QVERIFY(!bridge.hasChanges());
        }
    }
};

QTEST_MAIN(EngineTests)
#include "EngineTests.moc"

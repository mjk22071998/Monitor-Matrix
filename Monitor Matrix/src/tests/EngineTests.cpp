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

    void testRunLayoutValidationAndTrigger() {
        ScreenDropEngine engine;
        PlacementModel placements;
        AppCatalogModel catalog;
        MonitorCanvasModel monitors;

        engine.setPlacementModel(&placements);
        engine.setCatalogModel(&catalog);
        engine.setCanvasModel(&monitors);

        QSignalSpy notifySpy(&engine, &ScreenDropEngine::notificationRequested);
        QSignalSpy runningSpy(&engine, &ScreenDropEngine::isRunningChanged);

        // 1. Run layout on empty placements should notify error and not run
        engine.runLayout();
        QVERIFY(!engine.isRunning());
        QCOMPARE(runningSpy.count(), 0);
        QCOMPARE(notifySpy.count(), 1);
        QCOMPARE(notifySpy.last().at(0).toString(), QStringLiteral("Empty Layout"));
        QCOMPARE(notifySpy.last().at(2).toBool(), true); // isError = true

        // 2. Run layout on overlapping placements should notify error and not run
        placements.placeApp("app1", "App 1", "\\.\\DISPLAY1", 0, QRectF(0, 0, 200, 200));
        placements.placeApp("app2", "App 2", "\\.\\DISPLAY1", 0, QRectF(50, 50, 200, 200));
        QVERIFY(engine.hasOverlaps());

        engine.runLayout();
        QVERIFY(!engine.isRunning());
        QCOMPARE(runningSpy.count(), 0);
        QCOMPARE(notifySpy.last().at(0).toString(), QStringLiteral("Invalid Layout"));
        QCOMPARE(notifySpy.last().at(2).toBool(), true); // isError = true

        // 3. Clear overlap and run valid layout
        placements.clearAll();
        placements.placeApp("app1", "App 1", "\\.\\DISPLAY1", 0, QRectF(0, 0, 200, 200));
        QVERIFY(!engine.hasOverlaps());

        engine.runLayout();
        QVERIFY(engine.isRunning());
        QVERIFY(runningSpy.count() >= 1);
        QCOMPARE(notifySpy.last().at(0).toString(), QStringLiteral("Running"));
        QCOMPARE(notifySpy.last().at(2).toBool(), false); // isError = false
    }

    void testLegacyLayoutCompatibility() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ScreenDropEngine engine;
        PlacementModel placements;
        AppCatalogModel catalog;
        MonitorCanvasModel monitors;

        engine.setPlacementModel(&placements);
        engine.setCatalogModel(&catalog);
        engine.setCanvasModel(&monitors);

        // 1. Version 1 legacy format
        QString v1Path = tempDir.filePath("legacy_v1.sav");
        {
            QFile file(v1Path);
            QVERIFY(file.open(QIODevice::WriteOnly));
            QDataStream stream(&file);
            stream.setVersion(QDataStream::Qt_6_2);
            stream << quint32(0x53445250); // SDRP
            stream << quint32(1);          // version 1
            stream << quint32(1);          // monitor count
            stream << QString("\\.\\DISPLAY1");
            stream << qint32(0) << qint32(0) << qint32(1920) << qint32(1080);
            stream << quint32(1);          // placement count
            stream << QString("legacyV1App"); // appId
            stream << QString("\\.\\DISPLAY1");
            stream << qint32(0) << qint32(0) << qint32(1920) << qint32(1080);
            stream << quint32(0);          // position FULL_SCREEN
        }

        QVERIFY(engine.loadLayout(v1Path));
        QCOMPARE(placements.count(), 1);
        QCOMPARE(placements.get(0)["appId"].toString(), QStringLiteral("legacyV1App"));
        QCOMPARE(placements.get(0)["targetDeviceName"].toString(), QStringLiteral("\\.\\DISPLAY1"));

        // 2. Version 2 legacy format (with customRect)
        QString v2Path = tempDir.filePath("legacy_v2.sav");
        {
            QFile file(v2Path);
            QVERIFY(file.open(QIODevice::WriteOnly));
            QDataStream stream(&file);
            stream.setVersion(QDataStream::Qt_6_2);
            stream << quint32(0x53445250); // SDRP
            stream << quint32(2);          // version 2
            stream << quint32(1);          // monitor count
            stream << QString("\\.\\DISPLAY1");
            stream << qint32(0) << qint32(0) << qint32(1920) << qint32(1080);
            stream << quint32(1);          // placement count
            stream << QString("legacyV2App");
            stream << QString("\\.\\DISPLAY1");
            stream << qint32(0) << qint32(0) << qint32(1920) << qint32(1080);
            stream << qint32(0) << qint32(0) << qint32(1920) << qint32(1040);
            stream << quint32(0);          // position
            stream << quint8(1);           // hasCustom = 1
            stream << qint32(100) << qint32(100) << qint32(500) << qint32(400);
        }

        QVERIFY(engine.loadLayout(v2Path));
        QCOMPARE(placements.count(), 1);
        QCOMPARE(placements.get(0)["appId"].toString(), QStringLiteral("legacyV2App"));
        QRectF r2 = placements.placementRect(placements.get(0)["placementId"].toString());
        QCOMPARE(r2, QRectF(100, 100, 500, 400));

        // 3. Version 3 legacy format (with placementId)
        QString v3Path = tempDir.filePath("legacy_v3.sav");
        {
            QFile file(v3Path);
            QVERIFY(file.open(QIODevice::WriteOnly));
            QDataStream stream(&file);
            stream.setVersion(QDataStream::Qt_6_2);
            stream << quint32(0x53445250); // SDRP
            stream << quint32(3);          // version 3
            stream << quint32(1);          // monitor count
            stream << QString("\\.\\DISPLAY1");
            stream << qint32(0) << qint32(0) << qint32(1920) << qint32(1080);
            stream << quint32(1);          // placement count
            stream << QString("customPlacementId3");
            stream << QString("legacyV3App");
            stream << QString("\\.\\DISPLAY1");
            stream << qint32(0) << qint32(0) << qint32(1920) << qint32(1080);
            stream << qint32(0) << qint32(0) << qint32(1920) << qint32(1040);
            stream << quint32(0);          // position
            stream << quint8(1);           // hasCustom = 1
            stream << qint32(50) << qint32(50) << qint32(300) << qint32(300);
        }

        QVERIFY(engine.loadLayout(v3Path));
        QCOMPARE(placements.count(), 1);
        QCOMPARE(placements.get(0)["placementId"].toString(), QStringLiteral("customPlacementId3"));
        QCOMPARE(placements.get(0)["appId"].toString(), QStringLiteral("legacyV3App"));
        QRectF r3 = placements.placementRect("customPlacementId3");
        QCOMPARE(r3, QRectF(50, 50, 300, 300));
    }

    void testSavePreservesPlacementAttributes() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QString layoutPath = tempDir.filePath("attributes_test.sav");

        ScreenDropEngine engine;
        PlacementModel placements;
        AppCatalogModel catalog;
        MonitorCanvasModel monitors;

        engine.setPlacementModel(&placements);
        engine.setCatalogModel(&catalog);
        engine.setCanvasModel(&monitors);

        QString placedId = placements.placeApp("testAppAttr", "Attr App", "\\.\\DISPLAY1", 2, QRectF(120, 80, 450, 350));
        QVERIFY(!placedId.isEmpty());

        QVERIFY(engine.saveLayout(layoutPath));
        QVERIFY(QFile::exists(layoutPath));

        placements.clearAll();
        QCOMPARE(placements.count(), 0);

        QVERIFY(engine.loadLayout(layoutPath));
        QCOMPARE(placements.count(), 1);

        QVariantMap item = placements.get(0);
        QCOMPARE(item["appId"].toString(), QStringLiteral("testAppAttr"));
        QCOMPARE(item["targetDeviceName"].toString(), QStringLiteral("\\.\\DISPLAY1"));
        QCOMPARE(item["positionEnum"].toInt(), 2);
        QRectF loadedRect = placements.placementRect(item["placementId"].toString());
        QCOMPARE(loadedRect, QRectF(120, 80, 450, 350));
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

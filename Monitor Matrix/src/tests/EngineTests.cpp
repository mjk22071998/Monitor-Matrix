#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include "../controllers/ScreenDropEngine.h"
#include "../controllers/DisplayConfigBridge.h"
#include "../models/qml/AppCatalogModel.h"
#include "../models/qml/MonitorCanvasModel.h"
#include "../models/qml/PlacementModel.h"
#include "../services/LaunchWindowPlacement.h"
#include "../platform/windows/WindowsDisplayTopology.h"

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

    void test4KAboveDual1080pPlacementMapping() {
        // Topology: 4K screen (3840x2160) positioned directly above two 1080p screens.
        // Screen 2 (1080p bottom-left) is Primary at (0, 0, 1920, 1080).
        // Screen 3 (1080p bottom-right) is at (1920, 0, 1920, 1080).
        // Screen 1 (4K top) is at (0, -2160, 3840, 2160).
        WindowsNativeMonitor native4K;
        native4K.deviceName = QStringLiteral("\\\\.\\DISPLAY1");
        native4K.monitorRect = QRect(0, -2160, 3840, 2160);
        native4K.workRect = QRect(0, -2160, 3840, 2160);
        native4K.primary = false;

        // 1. Top-Left Corner
        QRect tlZone(0, 0, 1920, 1080);
        QRect tlMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, tlZone);
        QCOMPARE(tlMapped, QRect(0, -2160, 1920, 1080));
        QVERIFY(tlMapped.bottom() <= 0); // Must NOT bleed into lower screens (y >= 0)

        // 2. Top-Right Corner
        QRect trZone(1920, 0, 1920, 1080);
        QRect trMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, trZone);
        QCOMPARE(trMapped, QRect(1920, -2160, 1920, 1080));
        QVERIFY(trMapped.bottom() <= 0);

        // 3. Bottom-Left Corner
        QRect blZone(0, 1080, 1920, 1080);
        QRect blMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, blZone);
        QCOMPARE(blMapped, QRect(0, -1080, 1920, 1080));
        QVERIFY(blMapped.bottom() <= 0); // Must end at y=0, exactly touching edge without bleeding into lower screens!

        // 4. Bottom-Right Corner
        QRect brZone(1920, 1080, 1920, 1080);
        QRect brMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, brZone);
        QCOMPARE(brMapped, QRect(1920, -1080, 1920, 1080));
        QVERIFY(brMapped.bottom() <= 0);
        QVERIFY(brMapped.right() <= 3840);
    }

    void test4KPrimaryAboveDual1080pPlacementMapping() {
        // Topology: 4K screen (3840x2160) is Primary at (0, 0, 3840, 2160) with 48px taskbar.
        // Below it are two 1080p screens at (0, 2160, 1920, 1080) and (1920, 2160, 1920, 1080).
        WindowsNativeMonitor native4K;
        native4K.deviceName = QStringLiteral("\\\\.\\DISPLAY1");
        native4K.monitorRect = QRect(0, 0, 3840, 2160);
        native4K.workRect = QRect(0, 0, 3840, 2112); // Work rect excluding taskbar
        native4K.primary = true;

        // 1. Top-Left Corner
        QRect tlZone(0, 0, 1920, 1080);
        QRect tlMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, tlZone);
        QCOMPARE(tlMapped.x(), 0);
        QCOMPARE(tlMapped.y(), 0);
        QCOMPARE(tlMapped.width(), 1920);
        QCOMPARE(tlMapped.height(), 1056);
        QVERIFY(tlMapped.bottom() <= 2112); // Strictly within workRect

        // 2. Top-Right Corner
        QRect trZone(1920, 0, 1920, 1080);
        QRect trMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, trZone);
        QCOMPARE(trMapped.x(), 1920);
        QCOMPARE(trMapped.y(), 0);
        QCOMPARE(trMapped.width(), 1920);
        QCOMPARE(trMapped.height(), 1056);
        QVERIFY(trMapped.bottom() <= 2112);

        // 3. Bottom-Left Corner
        QRect blZone(0, 1080, 1920, 1080);
        QRect blMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, blZone);
        QCOMPARE(blMapped.x(), 0);
        QCOMPARE(blMapped.y(), 1056);
        QCOMPARE(blMapped.width(), 1920);
        QCOMPARE(blMapped.height(), 1056);
        QVERIFY(blMapped.bottom() <= 2112); // Never spans past taskbar or into lower screens!

        // 4. Bottom-Right Corner
        QRect brZone(1920, 1080, 1920, 1080);
        QRect brMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, brZone);
        QCOMPARE(brMapped.x(), 1920);
        QCOMPARE(brMapped.y(), 1056);
        QCOMPARE(brMapped.width(), 1920);
        QCOMPARE(brMapped.height(), 1056);
        QVERIFY(brMapped.bottom() <= 2112);
    }

    void test4KCenteredAboveDual1080pPlacementMapping() {
        // Topology: 4K screen centered above at (-960, -2160, 3840, 2160)
        WindowsNativeMonitor native4K;
        native4K.deviceName = QStringLiteral("\\\\.\\DISPLAY1");
        native4K.monitorRect = QRect(-960, -2160, 3840, 2160);
        native4K.workRect = QRect(-960, -2160, 3840, 2160);
        native4K.primary = false;

        // Bottom-Right Corner (local: 1920, 1080, 1920, 1080)
        QRect brZone(1920, 1080, 1920, 1080);
        QRect brMapped = LaunchWindowPlacement::mapZoneToNativeWorkArea(native4K, brZone);
        QCOMPARE(brMapped, QRect(960, -1080, 1920, 1080));
        QVERIFY(brMapped.left() >= -960);
        QVERIFY(brMapped.right() <= 2880);
        QVERIFY(brMapped.bottom() <= 0); // No bleed into lower monitors
    }

    void testPlacementModelCaseInsensitiveMatchingAnd4KBounds() {
        PlacementModel model;
        QList<MonitorInfo> mons;

        MonitorInfo m4k;
        m4k.deviceName = QStringLiteral("\\\\.\\DISPLAY1");
        m4k.geometry = QRect(0, -2160, 3840, 2160);
        m4k.availableGeometry = m4k.geometry;
        mons.append(m4k);

        MonitorInfo m1080_1;
        m1080_1.deviceName = QStringLiteral("\\\\.\\DISPLAY2");
        m1080_1.geometry = QRect(0, 0, 1920, 1080);
        m1080_1.availableGeometry = m1080_1.geometry;
        mons.append(m1080_1);

        model.setMonitors(mons);

        // Place app with lowercase device name
        QString id = model.placeApp("app4k", "App 4K", "\\\\.\\display1", 8, QRectF(1920, 1080, 1920, 1080));
        QVERIFY(!id.isEmpty());

        QRectF rect = model.placementRect(id);
        QCOMPARE(rect, QRectF(1920, 1080, 1920, 1080));

        // toPlacements should match case-insensitively and preserve monitor geometry
        QList<Placement> placements = model.toPlacements();
        QCOMPARE(placements.size(), 1);
        QCOMPARE(placements.first().monitor.geometry, QRect(0, -2160, 3840, 2160));
    }
};

QTEST_MAIN(EngineTests)
#include "EngineTests.moc"

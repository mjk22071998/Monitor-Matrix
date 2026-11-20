#include <QTest>
#include <QRectF>
#include "../models/qml/AppCatalogModel.h"
#include "../models/qml/MonitorCanvasModel.h"
#include "../models/qml/PlacementModel.h"

class ModelUnitTests : public QObject {
    Q_OBJECT
private slots:
    // Review Focus 2: Invalid app entry rejection
    void testAppCatalogValidation() {
        AppCatalogModel catalog;
        QVERIFY(!catalog.addApplication("", ""));
        QVERIFY(!catalog.addApplication("non_existent_file.exe", "Valid Name"));
        QVERIFY(!catalog.addUrl("not_a_valid_url", "Test"));
        QVERIFY(!catalog.addUrl("ftp://example.com", "Test"));
        QVERIFY(!catalog.addUrl("https://example.com", ""));
    }

    // Review Focus 3: Resize clamp to MIN_SIZE
    void testPlacementModelMinSizeClamp() {
        PlacementModel placements;
        QString pId = placements.placeApp("app1", "App 1", "\\.\\DISPLAY1", 0, QRectF(10, 10, 200, 200));
        QVERIFY(!pId.isEmpty());
        // Attempt to collapse to 10x10 px (below MIN_SIZE 40)
        placements.updatePlacementRect(pId, QRectF(10, 10, 10, 10));
        QRectF rect = placements.placementRect(pId);
        QCOMPARE(rect.width(), 40.0);
        QCOMPARE(rect.height(), 40.0);
    }

    // Review Focus 5: Overlap calculation
    void testPlacementOverlapDetection() {
        PlacementModel placements;
        QString id1 = placements.placeApp("app1", "App 1", "\\.\\DISPLAY1", 0, QRectF(0, 0, 100, 100));
        QString id2 = placements.placeApp("app2", "App 2", "\\.\\DISPLAY1", 0, QRectF(50, 50, 100, 100));
        QVERIFY(placements.hasOverlaps());
        QCOMPARE(placements.invalidCount(), 2);
    }

    void testPlacementDifferentMonitorsNoOverlap() {
        PlacementModel placements;
        QString id1 = placements.placeApp("app1", "App 1", "\\.\\DISPLAY1", 0, QRectF(0, 0, 100, 100));
        QString id2 = placements.placeApp("app2", "App 2", "\\.\\DISPLAY2", 0, QRectF(0, 0, 100, 100));
        QVERIFY(!placements.hasOverlaps());
        QCOMPARE(placements.invalidCount(), 0);
    }

    void testPlacementRemovalAndClear() {
        PlacementModel placements;
        QString id1 = placements.placeApp("app1", "App 1", "\\.\\DISPLAY1", 0, QRectF(0, 0, 100, 100));
        QString id2 = placements.placeApp("app2", "App 2", "\\.\\DISPLAY1", 0, QRectF(50, 50, 100, 100));
        QVERIFY(placements.hasOverlaps());
        QCOMPARE(placements.invalidCount(), 2);

        QVERIFY(placements.removePlacement(id2));
        QCOMPARE(placements.count(), 1);
        QVERIFY(!placements.hasOverlaps());
        QCOMPARE(placements.invalidCount(), 0);

        placements.clearAll();
        QCOMPARE(placements.count(), 0);
        QVERIFY(!placements.hasOverlaps());
    }

    void testPlacementBoundaryConstraints() {
        PlacementModel placements;
        placements.setMonitorBounds("\\.\\DISPLAY1", QRectF(0, 0, 1920, 1080));

        QString id = placements.placeApp("app1", "App 1", "\\.\\DISPLAY1", 0, QRectF(1900, 1050, 100, 100));
        QRectF rect = placements.placementRect(id);
        // Bounded within 1920x1080: x + w <= 1920, y + h <= 1080
        QVERIFY(rect.right() <= 1920.0);
        QVERIFY(rect.bottom() <= 1080.0);
        QVERIFY(rect.width() >= 40.0);
        QVERIFY(rect.height() >= 40.0);
    }

    void testAppCatalogFilteringAndPlacement() {
        AppCatalogModel catalog;
        QVERIFY(catalog.addUrl("https://example.com/one", "Alpha One"));
        QVERIFY(catalog.addUrl("https://example.com/two", "Beta Two"));
        QCOMPARE(catalog.count(), 2);

        catalog.setSearchFilter("Alpha");
        QCOMPARE(catalog.count(), 1);
        QCOMPARE(catalog.data(catalog.index(0, 0), AppCatalogModel::NameRole).toString(), QString("Alpha One"));

        catalog.setSearchFilter("GammaNonExistent");
        QCOMPARE(catalog.count(), 0);

        catalog.setSearchFilter("");
        QCOMPARE(catalog.count(), 2);

        // Check isPlaced tracking
        QString firstId = catalog.data(catalog.index(0, 0), AppCatalogModel::IdRole).toString();
        QVERIFY(!catalog.isPlaced(firstId));
        catalog.setPlacedAppIds({ firstId });
        QVERIFY(catalog.isPlaced(firstId));
        QVERIFY(catalog.data(catalog.index(0, 0), AppCatalogModel::IsPlacedRole).toBool());
    }

    void testMonitorCanvasModel() {
        MonitorCanvasModel canvasModel;
        QList<MonitorInfo> monitors;
        MonitorInfo m1;
        m1.deviceName = "\\.\\DISPLAY1";
        m1.model = "Main Display";
        m1.geometry = QRect(0, 0, 1920, 1080);
        m1.availableGeometry = QRect(0, 0, 1920, 1040);
        m1.primary = true;
        monitors.append(m1);
        canvasModel.setMonitors(monitors);

        QCOMPARE(canvasModel.rowCount(), 1);
        QModelIndex idx = canvasModel.index(0, 0);
        QCOMPARE(canvasModel.data(idx, MonitorCanvasModel::DeviceNameRole).toString(), QString("\\.\\DISPLAY1"));
        QCOMPARE(canvasModel.data(idx, MonitorCanvasModel::RectWidthRole).toReal(), 1920.0);
        QCOMPARE(canvasModel.data(idx, MonitorCanvasModel::IsPrimaryRole).toBool(), true);

        QVariantMap map = canvasModel.getMonitorByName("\\.\\DISPLAY1");
        QCOMPARE(map.value("modelName").toString(), QString("Main Display"));
    }
};

QTEST_MAIN(ModelUnitTests)
#include "ModelUnitTests.moc"

#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QList>

#include "../services/LayoutLaunchService.h"
#include "../services/AppCatalogService.h"
#include "../managers/LayoutManager.h"

class AppCatalogModel;
class MonitorCanvasModel;
class PlacementModel;

class ScreenDropEngine : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentFileName READ currentFileName NOTIFY currentFileNameChanged)
    Q_PROPERTY(QString currentFilePath READ currentFilePath WRITE setCurrentFilePath NOTIFY currentFilePathChanged)
    Q_PROPERTY(bool isDirty READ isDirty NOTIFY dirtyChanged)
    Q_PROPERTY(bool isLayoutDirty READ isDirty NOTIFY dirtyChanged)
    Q_PROPERTY(bool hasOverlaps READ hasOverlaps NOTIFY hasOverlapsChanged)
    Q_PROPERTY(int invalidCount READ invalidCount NOTIFY invalidCountChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(bool darkTheme READ darkTheme WRITE setDarkTheme NOTIFY darkThemeChanged)

public:
    explicit ScreenDropEngine(QObject* parent = nullptr);
    ScreenDropEngine(
        AppCatalogModel* catalogModel,
        MonitorCanvasModel* canvasModel,
        PlacementModel* placementModel,
        QObject* parent = nullptr);
    ~ScreenDropEngine() override = default;

    QString currentFileName() const;
    QString currentFilePath() const { return m_currentFilePath; }
    void setCurrentFilePath(const QString& path);

    bool isDirty() const { return m_isDirty; }
    void setDirty(bool dirty);

    bool hasOverlaps() const { return m_hasOverlaps; }
    int invalidCount() const { return m_invalidCount; }
    bool isRunning() const { return m_isRunning; }
    void setIsRunning(bool running);

    bool darkTheme() const;
    void setDarkTheme(bool dark);

    void setModels(AppCatalogModel* catalog, MonitorCanvasModel* canvas, PlacementModel* placements);
    void setCatalogModel(AppCatalogModel* model);
    void setCanvasModel(MonitorCanvasModel* model);
    void setPlacementModel(PlacementModel* model);

    AppCatalogModel* catalogModel() const { return m_catalogModel; }
    MonitorCanvasModel* canvasModel() const { return m_canvasModel; }
    PlacementModel* placementModel() const { return m_placementModel; }

    Q_INVOKABLE void runLayout();
    Q_INVOKABLE bool saveLayout(const QUrl& fileUrl);
    Q_INVOKABLE bool saveLayout(const QString& filePath);
    Q_INVOKABLE bool saveCurrentLayout();
    Q_INVOKABLE bool loadLayout(const QUrl& fileUrl);
    Q_INVOKABLE bool loadLayout(const QString& filePath);
    Q_INVOKABLE void newLayout();
    Q_INVOKABLE void openLogsFolder();

    Q_INVOKABLE void markDirty();
    Q_INVOKABLE void markClean();

    // Review Focus 4: Mismatch simulation for testing
    Q_INVOKABLE void simulateMismatchLoad();

    Q_INVOKABLE void notify(const QString& title, const QString& message, bool isError = false);

signals:
    void currentFileNameChanged();
    void currentFilePathChanged();
    void dirtyChanged(bool isDirty);
    void hasOverlapsChanged(bool hasOverlaps);
    void invalidCountChanged(int invalidCount);
    void isRunningChanged(bool isRunning);
    void darkThemeChanged(bool darkTheme);
    void monitorMismatchDetected(int savedCount, int currentCount);
    void notificationRequested(const QString& title, const QString& message, bool isError);

private:
    QString m_currentFilePath;
    bool m_isDirty = false;
    bool m_hasOverlaps = false;
    int m_invalidCount = 0;
    bool m_isRunning = false;
    bool m_isLoading = false;

    AppCatalogModel* m_catalogModel = nullptr;
    MonitorCanvasModel* m_canvasModel = nullptr;
    PlacementModel* m_placementModel = nullptr;

    LayoutLaunchService m_launchService;
    AppCatalogService m_catalogService;
};

#include "ScreenDropEngine.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>

#include "../models/qml/AppCatalogModel.h"
#include "../models/qml/MonitorCanvasModel.h"
#include "../models/qml/PlacementModel.h"
#include "../platform/windows/WindowsDisplayTopology.h"
#include "../theme/AppTheme.h"
#include "../utils/AppLogger.h"

namespace {

QString normalizeFilePath(const QUrl& url)
{
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }
    const QString str = url.toString();
    if (str.startsWith(QStringLiteral("file:///"))) {
        return QUrl(str).toLocalFile();
    }
    return str;
}

QString normalizeFilePath(const QString& path)
{
    if (path.startsWith(QStringLiteral("file:///"))) {
        return QUrl(path).toLocalFile();
    }
    return path;
}

} // namespace

ScreenDropEngine::ScreenDropEngine(QObject* parent)
    : QObject(parent)
{
}

ScreenDropEngine::ScreenDropEngine(
    AppCatalogModel* catalogModel,
    MonitorCanvasModel* canvasModel,
    PlacementModel* placementModel,
    QObject* parent)
    : QObject(parent)
{
    setModels(catalogModel, canvasModel, placementModel);
}

QString ScreenDropEngine::currentFileName() const
{
    if (m_currentFilePath.isEmpty()) {
        return QStringLiteral("Untitled.sav");
    }
    return QFileInfo(m_currentFilePath).fileName();
}

void ScreenDropEngine::setCurrentFilePath(const QString& path)
{
    const QString normalized = normalizeFilePath(path);
    if (m_currentFilePath != normalized) {
        m_currentFilePath = normalized;
        emit currentFilePathChanged();
        emit currentFileNameChanged();
    }
}

void ScreenDropEngine::setDirty(bool dirty)
{
    if (m_isDirty != dirty) {
        m_isDirty = dirty;
        emit dirtyChanged(m_isDirty);
    }
}

void ScreenDropEngine::setIsRunning(bool running)
{
    if (m_isRunning != running) {
        m_isRunning = running;
        emit isRunningChanged(m_isRunning);
    }
}

bool ScreenDropEngine::darkTheme() const
{
    return AppTheme::mode() == AppThemeMode::Dark;
}

void ScreenDropEngine::setDarkTheme(bool dark)
{
    const AppThemeMode targetMode = dark ? AppThemeMode::Dark : AppThemeMode::Light;
    if (AppTheme::mode() != targetMode) {
        AppTheme::setMode(targetMode);
        emit darkThemeChanged(dark);
    }
}

void ScreenDropEngine::setModels(AppCatalogModel* catalog, MonitorCanvasModel* canvas, PlacementModel* placements)
{
    setCatalogModel(catalog);
    setCanvasModel(canvas);
    setPlacementModel(placements);
}

void ScreenDropEngine::setCatalogModel(AppCatalogModel* model)
{
    m_catalogModel = model;
}

void ScreenDropEngine::setCanvasModel(MonitorCanvasModel* model)
{
    if (m_canvasModel == model) {
        return;
    }

    if (m_canvasModel) {
        m_canvasModel->disconnect(this);
    }

    m_canvasModel = model;

    if (m_canvasModel) {
        connect(m_canvasModel, &MonitorCanvasModel::monitorsChanged, this, [this]() {
            if (m_placementModel) {
                m_placementModel->setMonitors(m_canvasModel->monitors());
            }
        });

        if (m_placementModel) {
            m_placementModel->setMonitors(m_canvasModel->monitors());
        }
    }
}

void ScreenDropEngine::setPlacementModel(PlacementModel* model)
{
    if (m_placementModel == model) {
        return;
    }

    if (m_placementModel) {
        m_placementModel->disconnect(this);
    }

    m_placementModel = model;

    if (m_placementModel) {
        if (m_canvasModel) {
            m_placementModel->setMonitors(m_canvasModel->monitors());
        }

        connect(m_placementModel, &PlacementModel::placementsChanged, this, [this]() {
            if (!m_isLoading) {
                markDirty();
            }
        });

        connect(m_placementModel, &PlacementModel::countChanged, this, [this]() {
            if (!m_isLoading) {
                markDirty();
            }
        });

        connect(m_placementModel, &PlacementModel::overlapStateChanged, this, [this]() {
            const bool overlaps = m_placementModel->hasOverlaps();
            const int invalid = m_placementModel->invalidCount();

            if (m_hasOverlaps != overlaps) {
                m_hasOverlaps = overlaps;
                emit hasOverlapsChanged(m_hasOverlaps);
            }

            if (m_invalidCount != invalid) {
                m_invalidCount = invalid;
                emit invalidCountChanged(m_invalidCount);
            }
        });

        m_hasOverlaps = m_placementModel->hasOverlaps();
        m_invalidCount = m_placementModel->invalidCount();
        emit hasOverlapsChanged(m_hasOverlaps);
        emit invalidCountChanged(m_invalidCount);
    }
}

void ScreenDropEngine::runLayout()
{
    if (m_hasOverlaps) {
        notify(QStringLiteral("Invalid Layout"),
               QStringLiteral("Cannot run layout with overlapping placements."),
               true);
        return;
    }

    if (!m_placementModel || m_placementModel->count() == 0) {
        notify(QStringLiteral("Empty Layout"),
               QStringLiteral("No placements exist to launch."),
               true);
        return;
    }

    setIsRunning(true);

    const QList<AppItem> apps = m_catalogModel ? m_catalogModel->apps() : m_catalogService.loadApplications();
    QList<Placement> placements = m_placementModel->toPlacements();
    if (m_canvasModel) {
        const QList<MonitorInfo> mons = m_canvasModel->monitors();
        for (Placement& p : placements) {
            for (const MonitorInfo& mi : mons) {
                if (mi.deviceName.compare(p.monitor.deviceName, Qt::CaseInsensitive) == 0) {
                    p.monitor = mi;
                    break;
                }
            }
        }
    }

    m_launchService.launchPlacements(apps, placements);

    notify(QStringLiteral("Running"),
           QStringLiteral("Launching layout applications..."),
           false);

    QTimer::singleShot(1500, this, [this]() {
        setIsRunning(false);
    });
}

bool ScreenDropEngine::saveLayout(const QUrl& fileUrl)
{
    return saveLayout(normalizeFilePath(fileUrl));
}

bool ScreenDropEngine::saveLayout(const QString& filePath)
{
    QString targetPath = normalizeFilePath(filePath);
    if (targetPath.isEmpty()) {
        notify(QStringLiteral("Error"), QStringLiteral("Invalid file path for saving layout."), true);
        return false;
    }

    if (!targetPath.endsWith(QStringLiteral(".sav"), Qt::CaseInsensitive)) {
        targetPath += QStringLiteral(".sav");
    }

    QList<Placement> placements = m_placementModel ? m_placementModel->toPlacements() : QList<Placement>();
    QList<MonitorInfo> monitors;
    if (m_canvasModel && !m_canvasModel->monitors().isEmpty()) {
        monitors = m_canvasModel->monitors();
    } else {
        monitors = WindowsDisplayTopology::getMonitors();
    }

    for (Placement& p : placements) {
        for (const MonitorInfo& mi : monitors) {
            if (mi.deviceName.compare(p.monitor.deviceName, Qt::CaseInsensitive) == 0) {
                p.monitor = mi;
                break;
            }
        }
    }

    const bool ok = LayoutManager::saveToFile(targetPath, placements, monitors);
    if (!ok) {
        notify(QStringLiteral("Save Failed"),
               QStringLiteral("Failed to write layout file: %1").arg(targetPath),
               true);
        return false;
    }

    m_currentFilePath = targetPath;
    setDirty(false);
    emit currentFilePathChanged();
    emit currentFileNameChanged();

    notify(QStringLiteral("Saved"),
           QStringLiteral("Layout saved to %1").arg(QFileInfo(targetPath).fileName()),
           false);
    return true;
}

bool ScreenDropEngine::saveCurrentLayout()
{
    if (m_currentFilePath.isEmpty()) {
        notify(QStringLiteral("Save Layout"),
               QStringLiteral("Please select a file to save the layout."),
               true);
        return false;
    }
    return saveLayout(m_currentFilePath);
}

bool ScreenDropEngine::loadLayout(const QUrl& fileUrl)
{
    return loadLayout(normalizeFilePath(fileUrl));
}

bool ScreenDropEngine::loadLayout(const QString& filePath)
{
    const QString targetPath = normalizeFilePath(filePath);
    if (targetPath.isEmpty()) {
        notify(QStringLiteral("Error"), QStringLiteral("Invalid file path for loading layout."), true);
        return false;
    }

    QList<Placement> loadedPlacements;
    QList<MonitorInfo> loadedMonitors;

    const bool ok = LayoutManager::loadFromFile(targetPath, loadedPlacements, loadedMonitors);
    if (!ok) {
        notify(QStringLiteral("Load Failed"),
               QStringLiteral("Failed to read layout file: %1").arg(targetPath),
               true);
        return false;
    }

    // Review Focus 4: Monitor mismatch detection
    int currentCount = 0;
    if (m_canvasModel && m_canvasModel->count() > 0) {
        currentCount = m_canvasModel->count();
    } else {
        currentCount = WindowsDisplayTopology::getMonitors().size();
    }

    if (currentCount > 0 && loadedMonitors.size() != currentCount) {
        emit monitorMismatchDetected(loadedMonitors.size(), currentCount);
        notify(QStringLiteral("Monitor Mismatch"),
               QStringLiteral("Layout saved for %1 display(s), but %2 detected.")
                   .arg(loadedMonitors.size())
                   .arg(currentCount),
               true);
    }

    m_isLoading = true;
    if (m_placementModel) {
        if (m_canvasModel && !m_canvasModel->monitors().isEmpty()) {
            m_placementModel->setMonitors(m_canvasModel->monitors());
        } else if (!loadedMonitors.isEmpty()) {
            m_placementModel->setMonitors(loadedMonitors);
        }
        const QList<AppItem> catalog = m_catalogModel ? m_catalogModel->apps() : m_catalogService.loadApplications();
        m_placementModel->loadPlacements(loadedPlacements, catalog);

        if (m_catalogModel) {
            m_catalogModel->setPlacedAppIds(m_placementModel->placedAppIds());
        }
    }
    m_isLoading = false;

    m_currentFilePath = targetPath;
    setDirty(false);
    emit currentFilePathChanged();
    emit currentFileNameChanged();

    notify(QStringLiteral("Loaded"),
           QStringLiteral("Layout loaded from %1").arg(QFileInfo(targetPath).fileName()),
           false);
    return true;
}

void ScreenDropEngine::newLayout()
{
    m_isLoading = true;
    if (m_placementModel) {
        m_placementModel->clearAll();
    }
    if (m_catalogModel) {
        m_catalogModel->setPlacedAppIds({});
    }
    m_isLoading = false;

    m_currentFilePath.clear();
    setDirty(false);
    emit currentFilePathChanged();
    emit currentFileNameChanged();

    notify(QStringLiteral("New Layout"), QStringLiteral("Cleared canvas for new layout."), false);
}

void ScreenDropEngine::openLogsFolder()
{
    const QString dir = AppLogger::logDirectory();
    QDir().mkpath(dir);
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void ScreenDropEngine::markDirty()
{
    setDirty(true);
}

void ScreenDropEngine::markClean()
{
    setDirty(false);
}

void ScreenDropEngine::simulateMismatchLoad()
{
    const int current = (m_canvasModel && m_canvasModel->count() > 0) ? m_canvasModel->count() : 1;
    const int saved = current + 1;
    emit monitorMismatchDetected(saved, current);
}

void ScreenDropEngine::notify(const QString& title, const QString& message, bool isError)
{
    emit notificationRequested(title, message, isError);
}

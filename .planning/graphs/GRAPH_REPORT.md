# Graph Report - Monitor Matrix  (2026-11-21)

## Corpus Check
- 74 files · ~100,909 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 8 file(s) not represented in the graph (top: .ttf 2, (none) 1, .qrc 1)

## Summary
- 1041 nodes · 1982 edges · 38 communities (27 shown, 11 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 165 edges (avg confidence: 0.86)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `768452ae`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- AppThemePalette
- MonitorTopologyScene
- WindowsDisplayTopology.cpp
- AppItem
- MonitorScene
- qdebug
- MainWindow.h
- MonitorDropItem
- ScreenConfigDialog
- LaunchProcess.cpp
- MainWindow.cpp
- AppLogger.cpp
- PlacedItem
- AppListView
- MonitorTopologyItem.cpp
- PlacedItemHandlePoints
- MainWindow
- LaunchService.cpp
- WindowsDisplayModes.cpp
- LayoutManager.cpp
- AppEditDialog.cpp
- Communities (34 total, 5 thin omitted)
- AppPosition
- .mouseMoveEvent
- snapAndBlockResizeAgainstOtherItems
- AppPosition
- MainWindowAppManagement.cpp
- Screen Drop
- README.md

## God Nodes (most connected - your core abstractions)
1. `AppThemePalette` - 94 edges
2. `PlacedItem` - 55 edges
3. `MainWindow` - 49 edges
4. `MonitorTopologyScene` - 46 edges
5. `ScreenConfigDialog` - 44 edges
6. `MonitorDropItem` - 43 edges
7. `MonitorScene` - 37 edges
8. `AppItem` - 30 edges
9. `Communities (37 total, 9 thin omitted)` - 29 edges
10. `AppListView` - 23 edges

## Surprising Connections (you probably didn't know these)
- `God Nodes (most connected - your core abstractions)` --references--> `AppItem`  [INFERRED]
  .planning/graphs/GRAPH_REPORT.md → Monitor Matrix/src/models/AppItem.h
- `God Nodes (most connected - your core abstractions)` --references--> `MonitorScene`  [INFERRED]
  .planning/graphs/GRAPH_REPORT.md → Monitor Matrix/src/scene/MonitorScene.h
- `God Nodes (most connected - your core abstractions)` --references--> `PlacedItem`  [INFERRED]
  .planning/graphs/GRAPH_REPORT.md → Monitor Matrix/src/scene/PlacedItem.h
- `God Nodes (most connected - your core abstractions)` --references--> `AppThemePalette`  [INFERRED]
  .planning/graphs/GRAPH_REPORT.md → Monitor Matrix/src/theme/AppTheme.h
- `God Nodes (most connected - your core abstractions)` --references--> `AppListView`  [INFERRED]
  .planning/graphs/GRAPH_REPORT.md → Monitor Matrix/src/views/AppListView.h

## Import Cycles
- None detected.

## Communities (38 total, 11 thin omitted)

### Community 0 - "AppThemePalette"
Cohesion: 0.02
Nodes (90): AppThemePalette, badgeFill, badgeText, danger, dialogBg, dialogButtonBg, dialogButtonBorder, dialogButtonDefaultBg (+82 more)

### Community 1 - "MonitorTopologyScene"
Cohesion: 0.07
Nodes (14): MonitorTopologyItem, MonitorTopologyScene, ALIGNMENT_SNAP_PX, CONNECTION_TOLERANCE, DISPLAY_SCALE, m_displays, m_dragItem, m_horizontalGuide (+6 more)

### Community 2 - "WindowsDisplayTopology.cpp"
Cohesion: 0.08
Nodes (31): addPreferredTargetResolution(), addResolution(), addTargetModeResolution(), applyDisplayConfigs(), displayConfigQueryFlags(), DisplayConfigSnapshot, modes, paths (+23 more)

### Community 3 - "AppItem"
Cohesion: 0.08
Nodes (26): attachLaunchStrategy(), ConfigManager, loadIcon(), makeUniqueId(), normalizedType(), AppItem, icon, id (+18 more)

### Community 4 - "MonitorScene"
Cohesion: 0.10
Nodes (8): MonitorScene, m_appNamesByPlacementId, m_hasOverlaps, m_monitors, m_placements, MonitorScene::MonitorScene(), private, MonitorScene::rebuildItems()

### Community 5 - "qdebug"
Cohesion: 0.06
Nodes (15): calculateTargetRect(), clampRectToBounds(), mapZoneToNativeWorkArea(), placeWindow(), rectToString(), repaintWindow(), setWindowRect(), ExecutableStrategy (+7 more)

### Community 6 - "MainWindow.h"
Cohesion: 0.08
Nodes (5): MonitorDropItem, DisplayConfigService, DisplayConfigService::DisplayConfigService(), public, LayoutLaunchService

### Community 7 - "MonitorDropItem"
Cohesion: 0.07
Nodes (13): MonitorDropItem, m_allPlacedItem, m_badgeBg, m_badgeText, m_dockTargetRects, m_dragging, m_hasDockHover, m_hoverDockPosition (+5 more)

### Community 8 - "ScreenConfigDialog"
Cohesion: 0.05
Nodes (34): DisplayConfigService, MonitorTopologyScene, QCheckBox, QComboBox, QGraphicsView, QLabel, QPushButton, ScreenConfigDialog (+26 more)

### Community 9 - "LaunchProcess.cpp"
Cohesion: 0.10
Nodes (21): commandLineFor(), directoryFromPath(), environmentVariable(), expandEnvironmentStrings(), fileExists(), fileNameFromPath(), findWindowByProcessId(), findWindowByProcessName() (+13 more)

### Community 10 - "MainWindow.cpp"
Cohesion: 0.10
Nodes (7): MainWindow::createMenus(), MainWindow::onRunClicked(), MainWindow::openLayout(), MainWindow::save(), MainWindow::saveAs(), MainWindow::updateThemeActions(), MainWindow::updateTitle()

### Community 11 - "AppLogger.cpp"
Cohesion: 0.06
Nodes (19): enablePerMonitorDpiAwareness(), main(), runTaskCallback(), WindowsTaskRunner::run(), MonitorService, m_monitors, MonitorService::MonitorService(), rectToString() (+11 more)

### Community 12 - "PlacedItem"
Cohesion: 0.06
Nodes (20): PlacedItem, CENTER_SNAP_PX, HANDLE_HIT_PX, HANDLE_MARGIN_ITEM, HANDLE_PX, m_activeHandle, m_appId, m_appName (+12 more)

### Community 13 - "AppListView"
Cohesion: 0.09
Nodes (11): ScreenConfigDialog::ScreenConfigDialog(), AppListView, AppListView::AppListView(), m_apps, p_addButton, p_removeButton, private, QEvent (+3 more)

### Community 14 - "MonitorTopologyItem.cpp"
Cohesion: 0.07
Nodes (16): MonitorTopologyItem, m_displayIndex, m_primary, m_title, MonitorTopologyItem::MonitorTopologyItem(), PlacedItem::PlacedItem(), AppTheme, AppThemeMode (+8 more)

### Community 15 - "PlacedItemHandlePoints"
Cohesion: 0.11
Nodes (12): cursorForPlacedItemHandle(), PlacedItemHandlePoints, bl, bm, br, ml, mr, tl (+4 more)

### Community 16 - "MainWindow"
Cohesion: 0.08
Nodes (16): MainWindow, m_appCatalog, m_apps, m_currentFile, m_layoutLauncher, m_placements, p_appListView, p_darkThemeAction (+8 more)

### Community 17 - "LaunchService.cpp"
Cohesion: 0.16
Nodes (7): LaunchedProcess, processId, started, chromePathFromRegistry(), findChromeWindowName(), LaunchService, MainWindow::addDroppedApplicationFiles()

### Community 18 - "WindowsDisplayModes.cpp"
Cohesion: 0.24
Nodes (11): addDisplaySettingsResolutions(), addResolution(), addWindowsProvidedResolutions(), applyDisplaySettingsApi(), bitsPerPixelFromDevMode(), displayChangeResultMessage(), findDisplayModeForSize(), findDisplayModeForSizeWithFlags() (+3 more)

### Community 19 - "LayoutManager.cpp"
Cohesion: 0.11
Nodes (10): LayoutManager, s_magicNumber, s_version, Placement, appId, customRect, monitor, placementId (+2 more)

### Community 21 - "AppEditDialog.cpp"
Cohesion: 0.08
Nodes (17): AppEditDialog, AppEditDialog::AppEditDialog(), p_addButton, p_browseButton, p_nameEdit, p_targetEdit, private, QComboBox (+9 more)

### Community 22 - "Communities (34 total, 5 thin omitted)"
Cohesion: 0.07
Nodes (29): Communities (37 total, 9 thin omitted), Community 0 - "AppThemePalette", Community 10 - "MainWindow.cpp", Community 11 - "AppLogger.cpp", Community 12 - "PlacedItem", Community 13 - "AppListView", Community 14 - "MonitorTopologyItem.cpp", Community 15 - "PlacedItemHandlePoints" (+21 more)

### Community 24 - "AppPosition"
Cohesion: 0.18
Nodes (10): AppPosition, BOTTOM_HALF, BOTTOM_LEFT, BOTTOM_RIGHT, FULL_SCREEN, LEFT_HALF, RIGHT_HALF, TOP_HALF (+2 more)

### Community 25 - ".mouseMoveEvent"
Cohesion: 0.38
Nodes (5): applyMonitorCenterSnap(), clampRectToBounds(), clampResizeRectToBounds(), enforceMinSizeForHandle(), resizedParentRectForHandle()

### Community 26 - "snapAndBlockResizeAgainstOtherItems"
Cohesion: 0.29
Nodes (4): horizontalRangesOverlap(), snapAndBlockResizeAgainstOtherItems(), snapValue(), verticalRangesOverlap()

### Community 28 - "AppPosition"
Cohesion: 0.20
Nodes (10): AppPosition, BOTTOM_HALF, BOTTOM_LEFT, BOTTOM_RIGHT, FULL_SCREEN, LEFT_HALF, RIGHT_HALF, TOP_HALF (+2 more)

### Community 29 - "MainWindowAppManagement.cpp"
Cohesion: 0.31
Nodes (4): MainWindow::addApplicationFile(), MainWindow::addUrlLink(), MainWindow::removeApp(), showCatalogMessage()

## Knowledge Gaps
- **248 isolated node(s):** `paths`, `modes`, `m_result`, `m_uninitialize`, `badgeFill` (+243 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 498 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **11 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `AppThemePalette` connect `AppThemePalette` to `ScreenConfigDialog`, `AppLogger.cpp`, `MonitorTopologyItem.cpp`?**
  _High betweenness centrality (0.170) - this node is a cross-community bridge._
- **Are the 3 inferred relationships involving `AppThemePalette` (e.g. with `God Nodes (most connected - your core abstractions)` and `Suggested Questions`) actually correct?**
  _`AppThemePalette` has 3 INFERRED edges - model-reasoned connections that need verification._
- **What connects `paths`, `modes`, `m_result` to the rest of the system?**
  _248 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `AppThemePalette` be split into smaller, more focused modules?**
  _Cohesion score 0.022222222222222223 - nodes in this community are weakly interconnected._
- **Why does `MainWindow` connect `MainWindow` to `AppItem`, `MonitorScene`, `MainWindow.h`, `ScreenConfigDialog`, `MainWindow.cpp`, `AppLogger.cpp`, `AppListView`, `MonitorTopologyItem.cpp`, `LayoutManager.cpp`, `AppEditDialog.cpp`, `MainWindowAppManagement.cpp`?**
  _High betweenness centrality (0.096) - this node is a cross-community bridge._
- **Are the 2 inferred relationships involving `PlacedItem` (e.g. with `God Nodes (most connected - your core abstractions)` and `Surprising Connections (you probably didn't know these)`) actually correct?**
  _`PlacedItem` has 2 INFERRED edges - model-reasoned connections that need verification._
- **Should `MonitorTopologyScene` be split into smaller, more focused modules?**
  _Cohesion score 0.0715846994535519 - nodes in this community are weakly interconnected._
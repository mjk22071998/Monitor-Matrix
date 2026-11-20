# Technical Design Document: Pure Qt Quick / QML Architecture (Option A)

## 1. System Architecture & Boundaries

The architecture strictly decouples C++ system logic from declarative QML presentation:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        Qt Quick Presentation                           │
│                                                                        │
│   ┌──────────────────┐  ┌──────────────────────────────────────────┐  │
│   │   AppSidebar     │  │             CanvasView (Viewport)        │  │
│   │  (Search & Drag) │  │  ┌────────────────────────────────────┐  │  │
│   │                  │  │  │   MonitorItem (Physical Display)   │  │  │
│   │                  │  │  │  ┌──────────────────────────────┐ │  │  │
│   │                  │  │  │  │  PlacedAppItem (8-Handles)   │ │  │  │
│   │                  │  │  │  └──────────────────────────────┘ │  │  │
│   │                  │  │  └────────────────────────────────────┘  │  │
│   └──────────────────┘  └──────────────────────────────────────────┘  │
│   ┌────────────────────────────────────────────────────────────────┐  │
│   │      HeaderBar (Run Layout, Save/Load, Status Pill, Theme)     │  │
│   │      ScreenConfigSheet (Interactive Display Arranger Drawer)   │  │
│   └────────────────────────────────────────────────────────────────┘  │
└───────────────────────────────────▲────────────────────────────────────┘
                                    │ Property Bindings & Signals
┌───────────────────────────────────▼────────────────────────────────────┐
│                       C++ QML Bridges & ViewModels                     │
│                                                                        │
│   • AppCatalogModel     (QAbstractListModel - apps, search, placed)    │
│   • MonitorCanvasModel  (QAbstractListModel - connected monitors)      │
│   • PlacementModel      (QAbstractListModel - placed zones & overlaps) │
│   • ScreenDropEngine    (QObject Controller - run, save, load, theme)  │
│   • DisplayConfigBridge (QObject Controller - topology & resolution)   │
└───────────────────────────────────▲────────────────────────────────────┘
                                    │ Native Win32 / Qt Services
┌───────────────────────────────────▼────────────────────────────────────┐
│                    Existing Core C++ Subsystems (Preserved)            │
│                                                                        │
│   • LayoutLaunchService & WindowsTaskRunner (Process/URL launcher)     │
│   • LayoutManager (.sav binary serialization & deserialization)       │
│   • MonitorService & WindowsDisplayTopology (Native monitor queries)   │
│   • DisplayConfigService & WindowsDisplayModes (Resolution/Topology)   │
│   • AppCatalogService & ConfigManager (apps.json persistence)          │
│   • AppLogger (Thread-safe rotating logger)                            │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. C++ ViewModels & QML Controllers

### 2.1 `AppCatalogModel` (`QAbstractListModel`)
Exposes the application catalog to QML:
* **Roles**:
  * `idRole` (`QString`): Unique app identifier.
  * `nameRole` (`QString`): Display name.
  * `typeRole` (`QString`): `"Executable"` or `"URL"`.
  * `targetRole` (`QString`): Executable path or web URL.
  * `isPlacedRole` (`bool`): Whether this app is currently placed on any monitor.
  * `iconSourceRole` (`QString`): Resolved native icon URL or fallback SVG.
* **Invokables**:
  * `addApplication(filePath, name)`: Validates and adds executable app.
  * `addUrl(url, name)`: Validates and adds web link.
  * `removeApplication(appId)`: Removes app from catalog and unplaces it.
  * `setSearchFilter(filterText)`: Reactive instant search filtering.

### 2.2 `MonitorCanvasModel` (`QAbstractListModel`)
Exposes connected displays for canvas rendering:
* **Roles**:
  * `deviceNameRole` (`QString`): e.g. `\\.\DISPLAY1`.
  * `modelNameRole` (`QString`): e.g. `"Dell U2720Q"`.
  * `rectX`, `rectY`, `rectWidth`, `rectHeight` (`qreal`): Virtual screen desktop geometry.
  * `availX`, `availY`, `availWidth`, `availHeight` (`qreal`): Usable work area (minus taskbars).
  * `isPrimaryRole` (`bool`): True if monitor is primary `(0,0)`.
  * `scaleFactorRole` (`qreal`): DPI scale factor.

### 2.3 `PlacementModel` (`QAbstractListModel`)
Manages placed application zones on the canvas with real-time overlap computation:
* **Roles**:
  * `placementIdRole` (`QString`): Unique UUID per placed zone.
  * `appIdRole`, `appNameRole`, `appTypeRole` (`QString`).
  * `targetDeviceNameRole` (`QString`): Associated monitor name.
  * `rectX`, `rectY`, `rectW`, `rectH` (`qreal`): Normalized relative or pixel bounds within monitor.
  * `positionEnumRole` (`int`): `AppPosition` enum (Full, Left Half, Right Half, Custom, etc.).
  * `hasOverlapRole` (`bool`): Dynamic collision detection flag.
* **Invokables**:
  * `placeApp(appId, deviceName, positionEnum, customRect)`: Inserts new placement.
  * `updatePlacementRect(placementId, newRect)`: Real-time update during resize/drag.
  * `removePlacement(placementId)`: Removes single placement.
  * `clearAll()`: Clears all canvas placements.

### 2.4 `ScreenDropEngine` (`QObject` Singleton / Context Property)
Coordinates application-wide operations:
* **Properties**:
  * `currentFilePath` (`QString`): Current active `.sav` layout file name.
  * `isLayoutDirty` (`bool`): Unsaved changes flag.
  * `hasOverlaps` (`bool`): True if any placed apps intersect on the canvas.
  * `invalidPlacementCount` (`int`): Number of colliding apps.
  * `isRunning` (`bool`): Launcher execution state.
  * `darkTheme` (`bool`): Reactive dark/light mode toggle.
* **Invokables**:
  * `runLayout()`: Asynchronously launches all placed apps and positions windows.
  * `saveLayout(fileUrl)` / `saveCurrentLayout()`: Persists placements to `.sav`.
  * `loadLayout(fileUrl)`: Deserializes layout, checks monitor topology match, and repopulates canvas.
  * `openLogsFolder()`: Opens Windows Explorer at `%APPDATA%/ScreenDrop/logs`.
* **Signals**:
  * `notificationRequested(title, message, isError)`: Triggers transient HUD toasts.
  * `monitorLayoutMismatchWarning(savedCount, currentCount)`: Prompts user when opening layout on different monitor setups.

---

## 3. UI/UX Design System ("Studio Matrix")

Applying the principles from `frontend-design` and `qt-ui-design`:

### 3.1 Color Palette & Visual Identity
* **Background / Base**: Deep Obsidian (`#080B11`) and Slate Canvas (`#0E131F`).
* **Surfaces**: Frosted dark glass panels (`#141B2B`, 92% opacity) with razor-thin borders (`#212D45`).
* **Primary / Action**: Electric Cyan (`#00E5FF` / `#06B6D4`) with subtle 8px neon drop glow on active controls.
* **Status Signifiers**:
  * Valid Layout: Emerald Green (`#10B981`).
  * Colliding / Invalid Overlap: Warning Amber & Crimson (`#F59E0B` / `#EF4444`).
  * App Type Tags: Executable (`#06B6D4`), URL (`#A78BFA`).
* **Monitor Bezels**: Deep metallic charcoal (`#1A2234`) with chamfered inner borders and crisp high-contrast display identification labels (`DISPLAY1: 2560x1440 @ 144Hz`).

### 3.2 Typography & Scale
* **Primary Sans**: DM Sans / Inter for UI controls, buttons, toolbars, and dialog titles.
* **Monospace Badges**: JetBrains Mono / Consolas for coordinate tags, resolution tags, and placement dimension badges (e.g., `1280 x 720 px`).

### 3.3 Interactive Canvas & Placed App Controls
* **8-Point Resize Handles**: 8 interactive corner and edge handles (`TopLeft`, `TopMid`, `TopRight`, `MidLeft`, etc.) with hover highlight.
* **Magnetic Snapping Guides**: Cyan snapping guidelines appear when moving or resizing within 10px of monitor centerlines or sibling app boundaries.
* **Overlap Indication**: When two placed apps collide, their border transitions from subtle cyan to warning amber/red (`#EF4444`), and the top status bar pill pulses.
* **Canvas Navigation**: Smooth zooming (mouse wheel) and panning (middle-click or drag on canvas background) with a floating "Fit to View" control.

---

## 4. QML Component Structure

```
Screen Drop/
├── qml/
│   ├── main.qml                     # Root ApplicationWindow, shortcut keys, toast host
│   ├── theme/
│   │   ├── Theme.qml                # Design tokens, color palette, typography, easing curves
│   │   └── qmldir                   # Singleton declaration
│   ├── components/
│   │   ├── HeaderBar.qml            # Top navigation bar: Run button, Save/Load, Overlap pill
│   │   ├── NotificationToast.qml    # Floating HUD toasts (save success, warnings, errors)
│   │   ├── OverlapStatusPill.qml    # Reactive pill showing "Layout Valid" or "N Overlaps"
│   │   └── CanvasControls.qml       # Floating Zoom In/Out, Reset Zoom, Fit to View buttons
│   ├── views/
│   │   ├── AppSidebar.qml           # Searchable application list, drag sources, "+ Add App"
│   │   ├── CanvasView.qml           # Zoomable/pannable multi-monitor virtual desktop
│   │   ├── MonitorItem.qml          # Monitor bezel, display tag, and DropArea
│   │   └── PlacedAppItem.qml        # Interactive placed app, 8 resize handles, snap guides
│   └── dialogs/
│       ├── AppEditModal.qml         # Modern modal dialog for adding Executable / URL
│       └── ScreenConfigSheet.qml    # Slide-out drawer for multi-monitor topology & resolution
```

---

## 5. Development Milestones & Verification

| Milestone | Deliverables | Verification Gates |
|---|---|---|
| **1. C++ Models & Engine** | Implement `AppCatalogModel`, `MonitorCanvasModel`, `PlacementModel`, `ScreenDropEngine`. Wire `main.cpp` with `QGuiApplication` + `QQmlApplicationEngine`. Link Qt Quick modules in `.vcxproj`. | Compiles with MSBuild `v145` (0 warnings, 0 errors). Unit tests verify model roles and data integrity. |
| **2. QML Theme & App Shell** | Create `Theme.qml`, `main.qml`, `HeaderBar.qml`, `NotificationToast.qml`. | App boots with modern slate dark theme, window controls, and keyboard shortcuts (`Ctrl+S`, `Ctrl+O`, `Ctrl+R`). |
| **3. App Sidebar & Drag-Drop** | Implement `AppSidebar.qml` with search filter, "+ Add App" dialog modal, and initiate QML drag events. | Apps filter smoothly; dragging an app provides live visual feedback. |
| **4. Multi-Monitor Canvas & Placed Apps** | Implement `CanvasView.qml`, `MonitorItem.qml`, and `PlacedAppItem.qml` with 8 resize handles, magnetic snapping guides, and collision detection. | Dropping apps places them on monitors; resizing/moving updates geometry; overlapping triggers red border and status pill update. |
| **5. Display Topology Arranger** | Implement `ScreenConfigSheet.qml` and `DisplayConfigBridge.cpp` allowing visual monitor rearrangement, primary toggle, and resolution change. | Displays can be rearranged; changes apply to Windows display topology. |
| **6. Integration & Verification** | Integrate `LayoutManager` (.sav save/load), `LayoutLaunchService` (Run Layout), full end-to-end testing, and cleanup of legacy widget code. | Complete build passes; saving, loading, and launching layouts verified end-to-end. |

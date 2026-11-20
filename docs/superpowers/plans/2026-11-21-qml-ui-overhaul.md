# Modern QML UI Overhaul Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Overhaul the ScreenDrop desktop application UI by replacing the legacy QWidget/QGraphicsScene interface with a modern, GPU-accelerated Qt Quick / QML user experience.

**Architecture:** Pure Qt Quick application using `QGuiApplication` and `QQmlApplicationEngine`. C++ exposes `QAbstractListModel` subclasses (`AppCatalogModel`, `MonitorCanvasModel`, `PlacementModel`) and `QObject` controllers (`ScreenDropEngine`, `DisplayConfigBridge`) to declarative QML components adhering to the "Studio Matrix" design tokens and strict unversioned Qt 6 imports.

**Tech Stack:** C++17/20, Qt 6.11.1 (QtQuick, QtQuickControls2, QtQml, QtQuickLayouts, QtTest), MSVC v145, Windows 10/11 x64, MSBuild.

**Spec:** `docs/superpowers/specs/2026-11-21-qml-ui-overhaul-design.md`

## Global Constraints

- Compiles cleanly on Windows 10/11 x64 using MSVC `v145` toolset with 0 errors and 0 warnings.
- Targets Qt 6.11.1 MSVC 2022 64-bit (`C:\Qt\6.11.1\msvc2022_64`).
- Strict unversioned QML imports (`import QtQuick`, `import QtQuick.Controls`, `import QtQuick.Layouts`).
- All QML files must validate cleanly through `qmllint.exe`.
- Binary compatibility preserved for layout files (`.sav`) and catalog files (`apps.json`).
- High-DPI per-monitor awareness V2 maintained throughout.

## Review Focus

1. **Unsaved changes guard**: Attempting to load another layout or close with unsaved placements must flag dirty state and guard against data loss.
   *Pinned to Task 3 (Step 1a).*
2. **Invalid app entry validation**: Adding an empty name, non-existent executable path, or malformed URL must return a descriptive error and not pollute `apps.json`.
   *Pinned to Task 2 (Step 1a).*
3. **Placed window minimum size and boundary constraint**: Resizing an app zone to inverted or zero dimensions must clamp at `MIN_SIZE = 40.0` px and stay inside monitor bounds.
   *Pinned to Task 2 (Step 1b).*
4. **Monitor topology mismatch on load**: Deserializing a layout from a different display arrangement must detect device mismatches and emit a warning signal without crashing.
   *Pinned to Task 3 (Step 1b).*
5. **Mixed-DPI coordinate consistency**: Placements normalized inside a monitor must compute correct aspect ratio and scale factors regardless of host DPI.
   *Pinned to Task 2 (Step 1c).*

---

### Task 1: Build System Configuration & Qt Quick Module Integration

**Files:**
- Modify: `Monitor Matrix/Screen Drop.vcxproj:109-142`
- Modify: `Monitor Matrix/Screen Drop.vcxproj.filters`
- Modify: `Monitor Matrix/Resource.qrc:1-16`
- Create: `Monitor Matrix/qml/main.qml`
- Test: `Monitor Matrix/src/tests/SanityTest.cpp`

**Interfaces:**
- Consumes: Existing MSBuild project structure.
- Produces: Project compiled with `QtModules` including `qml;quick;quickcontrols2;testlib`, loading QML via `Resource.qrc`.

- [ ] **Step 1: Write test verifying Qt Quick compilation and QML engine instantiation**

Create `Monitor Matrix/src/tests/SanityTest.cpp`:
```cpp
#include <QTest>
#include <QQmlApplicationEngine>

class SanityTest : public QObject {
    Q_OBJECT
private slots:
    void testQmlEngineInstantiates() {
        QQmlApplicationEngine engine;
        QVERIFY(engine.rootObjects().isEmpty());
    }
};

QTEST_MAIN(SanityTest)
#include "SanityTest.moc"
```

- [ ] **Step 2: Run test to verify it fails (missing Qt Quick / QTest modules)**

Run:
```powershell
MSBuild "Monitor Matrix\Screen Drop.sln" /t:"Screen Drop" /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145
```
Expected: FAIL due to missing QML/Quick includes or linker symbols.

- [ ] **Step 3: Update `Screen Drop.vcxproj` and `Resource.qrc` to include Qt Quick modules**

Update `Screen Drop.vcxproj`:
- In both `Debug|x64` and `Release|x64` `<QtSettings>`, set:
  `<QtModules>core;gui;widgets;qml;quick;quickcontrols2;testlib</QtModules>`
- Ensure Release `<PlatformToolset>` is `v145`.
- Register `qml/main.qml` in `Resource.qrc` under prefix `/qml`.

Create minimal valid `Monitor Matrix/qml/main.qml`:
```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    visible: true
    title: "Screen Drop"
}
```

- [ ] **Step 4: Run build and verify test passes**

Run:
```powershell
MSBuild "Monitor Matrix\Screen Drop.sln" /t:"Screen Drop" /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145
```
Expected: Build succeeds with 0 errors and 0 warnings.
Validate QML syntax:
```powershell
& "C:\Qt\6.11.1\msvc2022_64\bin\qmllint.exe" "Monitor Matrix\qml\main.qml"
```
Expected: 0 errors.

- [ ] **Step 5: Commit**

```powershell
git add "Monitor Matrix/Screen Drop.vcxproj" "Monitor Matrix/Resource.qrc" "Monitor Matrix/qml/main.qml"
git commit -m "build: configure Qt Quick and QML modules in MSBuild project"
```

---

### Task 2: Core Data Models (`AppCatalogModel`, `MonitorCanvasModel`, `PlacementModel`)

**Files:**
- Create: `Monitor Matrix/src/models/qml/AppCatalogModel.h`
- Create: `Monitor Matrix/src/models/qml/AppCatalogModel.cpp`
- Create: `Monitor Matrix/src/models/qml/MonitorCanvasModel.h`
- Create: `Monitor Matrix/src/models/qml/MonitorCanvasModel.cpp`
- Create: `Monitor Matrix/src/models/qml/PlacementModel.h`
- Create: `Monitor Matrix/src/models/qml/PlacementModel.cpp`
- Test: `Monitor Matrix/src/tests/ModelUnitTests.cpp`

**Interfaces:**
- Consumes: `AppCatalogService`, `MonitorService`, `Placement`, `MonitorInfo`, `AppItem`.
- Produces:
  - `AppCatalogModel`: Roles `id`, `name`, `type`, `target`, `isPlaced`, `iconSource`. Invokables `addApplication`, `addUrl`, `removeApplication`, `setSearchFilter`.
  - `MonitorCanvasModel`: Roles `deviceName`, `modelName`, `rectX`, `rectY`, `rectWidth`, `rectHeight`, `availX`, `availY`, `availWidth`, `availHeight`, `isPrimary`, `scaleFactor`.
  - `PlacementModel`: Roles `placementId`, `appId`, `appName`, `appType`, `targetDeviceName`, `rectX`, `rectY`, `rectW`, `rectH`, `positionEnum`, `hasOverlap`. Invokables `placeApp`, `updatePlacementRect`, `removePlacement`, `clearAll`.

- [ ] **Step 1: Write failing unit tests for models (including Review Focus items 2, 3, 5)**

Create `Monitor Matrix/src/tests/ModelUnitTests.cpp`:
```cpp
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
        QVERIFY(!catalog.addUrl("not_a_valid_url", "Test"));
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
};

QTEST_MAIN(ModelUnitTests)
#include "ModelUnitTests.moc"
```

- [ ] **Step 2: Run test to verify it fails**

Run:
```powershell
MSBuild "Monitor Matrix\Screen Drop.sln" /t:"Screen Drop" /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145
```
Expected: FAIL with compilation errors (classes not defined).

- [ ] **Step 3: Implement `AppCatalogModel`, `MonitorCanvasModel`, and `PlacementModel`**

Implement:
1. `AppCatalogModel` inheriting `QAbstractListModel`, backing with `QList<AppItem>` filtered by `m_filterText`, checking `isPlaced(appId)`.
2. `MonitorCanvasModel` inheriting `QAbstractListModel`, backing with `QList<MonitorInfo>` synced from `MonitorService`.
3. `PlacementModel` inheriting `QAbstractListModel`, backing with `QList<Placement>`, computing bounding boxes and pairwise rectangle intersections for `hasOverlap`.

- [ ] **Step 4: Run unit tests to verify they pass**

Run:
```powershell
MSBuild "Monitor Matrix\Screen Drop.sln" /t:"Screen Drop" /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145
```
Expected: Build succeeds and tests pass.

- [ ] **Step 5: Commit**

```powershell
git add "Monitor Matrix/src/models/qml/" "Monitor Matrix/src/tests/ModelUnitTests.cpp"
git commit -m "feat(models): implement QML models for catalog, monitors, and placements"
```

---

### Task 3: C++ Controllers & Bridges (`ScreenDropEngine`, `DisplayConfigBridge`)

**Files:**
- Create: `Monitor Matrix/src/controllers/ScreenDropEngine.h`
- Create: `Monitor Matrix/src/controllers/ScreenDropEngine.cpp`
- Create: `Monitor Matrix/src/controllers/DisplayConfigBridge.h`
- Create: `Monitor Matrix/src/controllers/DisplayConfigBridge.cpp`
- Modify: `Monitor Matrix/main.cpp`
- Test: `Monitor Matrix/src/tests/EngineTests.cpp`

**Interfaces:**
- Consumes: `PlacementModel`, `MonitorCanvasModel`, `AppCatalogModel`, `LayoutManager`, `LayoutLaunchService`, `DisplayConfigService`.
- Produces:
  - `ScreenDropEngine` QML singleton: properties `currentFileName`, `isDirty`, `hasOverlaps`, `invalidCount`, `isRunning`, `darkTheme`; invokables `runLayout()`, `saveLayout(url)`, `loadLayout(url)`, `openLogsFolder()`.
  - `DisplayConfigBridge` QML singleton: exposes native display modes, topology coordinates, and applies resolution/primary updates.

- [ ] **Step 1: Write unit tests for `ScreenDropEngine` (including Review Focus items 1 and 4)**

Create `Monitor Matrix/src/tests/EngineTests.cpp`:
```cpp
#include <QTest>
#include "../controllers/ScreenDropEngine.h"

class EngineTests : public QObject {
    Q_OBJECT
private slots:
    // Review Focus 1: Unsaved changes dirty state
    void testDirtyStateTracking() {
        ScreenDropEngine engine;
        QVERIFY(!engine.isDirty());
        engine.markDirty();
        QVERIFY(engine.isDirty());
    }

    // Review Focus 4: Monitor mismatch detection
    void testMonitorMismatchSignal() {
        ScreenDropEngine engine;
        QSignalSpy spy(&engine, &ScreenDropEngine::monitorMismatchDetected);
        engine.simulateMismatchLoad();
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(EngineTests)
#include "EngineTests.moc"
```

- [ ] **Step 2: Run test to verify it fails**

Run:
```powershell
MSBuild "Monitor Matrix\Screen Drop.sln" /t:"Screen Drop" /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145
```
Expected: FAIL due to missing `ScreenDropEngine`.

- [ ] **Step 3: Implement `ScreenDropEngine`, `DisplayConfigBridge`, and wire `main.cpp`**

- Implement `ScreenDropEngine`: connects to `LayoutLaunchService`, `LayoutManager`, and models; tracks `isDirty`; emits `notificationRequested`.
- Implement `DisplayConfigBridge`: wraps `DisplayConfigService` and `WindowsDisplayModes`.
- Update `main.cpp` to use `QGuiApplication`, register types with `qmlRegisterSingletonInstance` / `qmlRegisterType`, load `qrc:/qml/main.qml` with `QQmlApplicationEngine`.

- [ ] **Step 4: Run test to verify it passes**

Run:
```powershell
MSBuild "Monitor Matrix\Screen Drop.sln" /t:"Screen Drop" /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145
```
Expected: 0 errors, 0 warnings.

- [ ] **Step 5: Commit**

```powershell
git add "Monitor Matrix/src/controllers/" "Monitor Matrix/main.cpp" "Monitor Matrix/src/tests/EngineTests.cpp"
git commit -m "feat(controllers): add ScreenDropEngine, DisplayConfigBridge, and wire QQmlApplicationEngine"
```

---

### Task 4: QML Design Tokens, Theme & Application Shell

**Files:**
- Create: `Monitor Matrix/qml/theme/Theme.qml`
- Create: `Monitor Matrix/qml/theme/qmldir`
- Create: `Monitor Matrix/qml/components/HeaderBar.qml`
- Create: `Monitor Matrix/qml/components/OverlapStatusPill.qml`
- Create: `Monitor Matrix/qml/components/NotificationToast.qml`
- Modify: `Monitor Matrix/qml/main.qml`
- Modify: `Monitor Matrix/Resource.qrc`

**Interfaces:**
- Consumes: `ScreenDropEngine` properties (`darkTheme`, `hasOverlaps`, `invalidCount`, `isRunning`).
- Produces: Polished top-level UI shell with custom header bar, responsive action triggers (Run, Save, Load, Settings), overlap warning pill, and animated toast notifications.

- [ ] **Step 1: Write QML lint test**

Validate `qmldir` and component contracts before implementation.

- [ ] **Step 2: Implement `Theme.qml` and `qmldir`**

Create `Monitor Matrix/qml/theme/Theme.qml`:
- Define design tokens: `bgBase` (`#080B11`), `bgSurface` (`#141B2B`), `borderSubtle` (`#212D45`), `accentCyan` (`#00E5FF`), `accentEmerald` (`#10B981`), `accentAmber` (`#F59E0B`), `accentCrimson` (`#EF4444`).
- Standardize typography font sizes, spacing scale, and animation durations (180ms–240ms cubic easing).

Create `Monitor Matrix/qml/theme/qmldir`:
```
singleton Theme 1.0 Theme.qml
```

- [ ] **Step 3: Implement `HeaderBar.qml`, `OverlapStatusPill.qml`, `NotificationToast.qml`, and `main.qml`**

- `HeaderBar.qml`: Contains app title, current file indicator, Save/Save As/Open buttons, Screen Settings button, Theme toggle, Overlap Status Pill, and prominent glowing "Run Layout" button.
- `OverlapStatusPill.qml`: Displays green check when valid, pulses warning amber/crimson when `hasOverlaps` is true.
- `NotificationToast.qml`: Floating transient banner at top-right for save/load/error messages with slide and fade animation.
- `main.qml`: Instantiates `HeaderBar`, binds keyboard shortcuts (`Ctrl+S`, `Ctrl+O`, `Ctrl+R`), hosts `NotificationToast`.

- [ ] **Step 4: Run `qmllint` to verify zero QML errors**

Run:
```powershell
& "C:\Qt\6.11.1\msvc2022_64\bin\qmllint.exe" "Monitor Matrix\qml\theme\Theme.qml" "Monitor Matrix\qml\components\HeaderBar.qml" "Monitor Matrix\qml\components\OverlapStatusPill.qml" "Monitor Matrix\qml\components\NotificationToast.qml" "Monitor Matrix\qml\main.qml"
```
Expected: 0 errors, 0 warnings.

- [ ] **Step 5: Commit**

```powershell
git add "Monitor Matrix/qml/theme/" "Monitor Matrix/qml/components/" "Monitor Matrix/qml/main.qml" "Monitor Matrix/Resource.qrc"
git commit -m "feat(ui): implement Theme singleton, HeaderBar, OverlapStatusPill, and NotificationToast"
```

---

### Task 5: App Catalog Sidebar, Search Filter & Add/Edit Modal

**Files:**
- Create: `Monitor Matrix/qml/views/AppSidebar.qml`
- Create: `Monitor Matrix/qml/dialogs/AppEditModal.qml`
- Modify: `Monitor Matrix/qml/main.qml`
- Modify: `Monitor Matrix/Resource.qrc`

**Interfaces:**
- Consumes: `AppCatalogModel` (`appCatalogModel`), `Theme`.
- Produces: Collapsible/fixed left sidebar displaying catalog items with search filtering, active drag sources, "+ Add App" modal trigger, and context menu for app removal.

- [ ] **Step 1: Write QML lint verification step**

Prepare lint invocation for `AppSidebar.qml` and `AppEditModal.qml`.

- [ ] **Step 2: Implement `AppEditModal.qml`**

- Modern modal dialog supporting tab selection for "Executable" vs "Web URL".
- Text inputs for App Name and Target Path/URL.
- "Browse..." file dialog integration for local `.exe` files.
- Input validation highlighting required fields and URL formatting.

- [ ] **Step 3: Implement `AppSidebar.qml`**

- Header with search field and "+ Add" button.
- `ListView` using `appCatalogModel` as model.
- Delegate with app icon, name, placed indicator pill, and draggable item initiating QML `Drag`.
- Drag metadata carries `appId`, `appName`, and `appType`.

- [ ] **Step 4: Embed `AppSidebar` into `main.qml` and run `qmllint`**

Run:
```powershell
& "C:\Qt\6.11.1\msvc2022_64\bin\qmllint.exe" "Monitor Matrix\qml\views\AppSidebar.qml" "Monitor Matrix\qml\dialogs\AppEditModal.qml" "Monitor Matrix\qml\main.qml"
```
Expected: 0 errors, 0 warnings.

- [ ] **Step 5: Commit**

```powershell
git add "Monitor Matrix/qml/views/AppSidebar.qml" "Monitor Matrix/qml/dialogs/AppEditModal.qml" "Monitor Matrix/qml/main.qml" "Monitor Matrix/Resource.qrc"
git commit -m "feat(ui): add AppSidebar with live search, drag initiation, and AppEditModal"
```

---

### Task 6: Multi-Monitor Canvas View & Placed App Widgets

**Files:**
- Create: `Monitor Matrix/qml/views/CanvasView.qml`
- Create: `Monitor Matrix/qml/components/MonitorItem.qml`
- Create: `Monitor Matrix/qml/components/PlacedAppItem.qml`
- Create: `Monitor Matrix/qml/components/CanvasControls.qml`
- Modify: `Monitor Matrix/qml/main.qml`
- Modify: `Monitor Matrix/Resource.qrc`

**Interfaces:**
- Consumes: `MonitorCanvasModel`, `PlacementModel`, `Theme`.
- Produces: Interactive virtual desktop canvas supporting zoom/pan, physical monitor rendering with drop targets, 8-point resize handles, magnetic snapping guides, and visual collision borders.

- [ ] **Step 1: Write QML lint test step**

Prepare lint invocation for canvas components.

- [ ] **Step 2: Implement `PlacedAppItem.qml`**

- Rectangular item with app name, type badge, and close button.
- 8 resize handles (`TopLeft`, `TopMid`, `TopRight`, `MidLeft`, etc.) with hover highlight.
- Drag area for repositioning within monitor.
- Magnetic snapping to 50% centerline and edges.
- Visual warning border (`Theme.accentCrimson`) when `hasOverlap` is true.

- [ ] **Step 3: Implement `MonitorItem.qml`**

- Renders monitor bezel, metallic frame, resolution tag (`2560x1440`), and primary display badge.
- `DropArea` accepting dragged items from `AppSidebar` or other monitors.
- Calculates placement zone (Full, Left Half, Right Half, or custom drop coordinates).

- [ ] **Step 4: Implement `CanvasView.qml` and `CanvasControls.qml`**

- Zoomable and pannable `Flickable` canvas with dotted grid background.
- Floating controls: Zoom In, Zoom Out, Reset, Fit All Monitors.
- Embed into `main.qml` central area.

- [ ] **Step 5: Run `qmllint` and verify build**

Run:
```powershell
& "C:\Qt\6.11.1\msvc2022_64\bin\qmllint.exe" "Monitor Matrix\qml\views\CanvasView.qml" "Monitor Matrix\qml\components\MonitorItem.qml" "Monitor Matrix\qml\components\PlacedAppItem.qml" "Monitor Matrix\qml\components\CanvasControls.qml"
MSBuild "Monitor Matrix\Screen Drop.sln" /t:"Screen Drop" /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145
```
Expected: 0 errors, 0 warnings.

- [ ] **Step 6: Commit**

```powershell
git add "Monitor Matrix/qml/views/CanvasView.qml" "Monitor Matrix/qml/components/" "Monitor Matrix/Resource.qrc"
git commit -m "feat(ui): implement multi-monitor canvas, PlacedAppItem with 8 handles, and snapping"
```

---

### Task 7: Display Topology Arranger Drawer (`ScreenConfigSheet.qml`)

**Files:**
- Create: `Monitor Matrix/qml/dialogs/ScreenConfigSheet.qml`
- Modify: `Monitor Matrix/src/controllers/DisplayConfigBridge.h`
- Modify: `Monitor Matrix/src/controllers/DisplayConfigBridge.cpp`
- Modify: `Monitor Matrix/qml/main.qml`
- Modify: `Monitor Matrix/Resource.qrc`

**Interfaces:**
- Consumes: `DisplayConfigBridge` (list of displays, available resolutions, primary status).
- Produces: Slide-out drawer or overlay sheet allowing interactive monitor reordering, resolution dropdown selection, primary monitor checkbox, and live Apply button.

- [ ] **Step 1: Write QML lint test step**

Prepare lint check for `ScreenConfigSheet.qml`.

- [ ] **Step 2: Implement `ScreenConfigSheet.qml`**

- Drawer / modal overlay anchored to the right or full-screen sheet.
- Mini interactive topology preview where monitors can be dragged to change virtual positions.
- Settings panel with resolution `ComboBox`, "Set as Primary" `CheckBox`, and "Apply Display Settings" `Button`.
- Direct invocation of `displayConfigBridge.applySettings()`.

- [ ] **Step 3: Run `qmllint` on `ScreenConfigSheet.qml`**

Run:
```powershell
& "C:\Qt\6.11.1\msvc2022_64\bin\qmllint.exe" "Monitor Matrix\qml\dialogs\ScreenConfigSheet.qml"
```
Expected: 0 errors, 0 warnings.

- [ ] **Step 4: Commit**

```powershell
git add "Monitor Matrix/qml/dialogs/ScreenConfigSheet.qml" "Monitor Matrix/src/controllers/DisplayConfigBridge.*" "Monitor Matrix/Resource.qrc"
git commit -m "feat(ui): add ScreenConfigSheet for interactive display topology and resolution adjustment"
```

---

### Task 8: Layout Persistence, Launch Execution, Legacy Cleanup & Verification

**Files:**
- Modify: `Monitor Matrix/src/controllers/ScreenDropEngine.cpp`
- Modify: `Monitor Matrix/Screen Drop.vcxproj`
- Modify: `Monitor Matrix/Screen Drop.vcxproj.filters`
- Remove: Legacy QWidget dialogs and QGraphicsView scene files that are fully superseded (`src/views/MainWindow.*`, `src/scene/MonitorScene.*`, `src/scene/PlacedItem*`, `src/dialog/ScreenConfigDialog.*`, `src/dialog/AppEditDialog.*`)
- Test: Full End-to-End Build and Verification

**Interfaces:**
- Consumes: Complete Qt Quick UI and C++ controllers.
- Produces: Polished, streamlined application binary free of deprecated widget code.

- [ ] **Step 1: Verify layout save/load (.sav) and process launch end-to-end**

- Verify `.sav` files created by legacy versions open accurately in QML.
- Verify saving from QML preserves binary structure for backwards compatibility.
- Verify "Run Layout" correctly triggers `LayoutLaunchService::launchPlacements` into assigned screen zones.

- [ ] **Step 2: Remove deprecated QWidget files from `.vcxproj` and filesystem**

Remove superseded files (`MainWindow.h/cpp`, `MainWindowAppManagement.cpp`, `MonitorScene.h/cpp`, `MonitorDropItem.h/cpp`, `PlacedItem*.h/cpp`, `ScreenConfigDialog.h/cpp`, `AppEditDialog.h/cpp`, `styles/app.qss`).
Update `Screen Drop.vcxproj` and `Screen Drop.vcxproj.filters` cleanly.

- [ ] **Step 3: Run full Release build with MSBuild**

Run:
```powershell
MSBuild "Monitor Matrix\Screen Drop.sln" /t:Rebuild /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145
```
Expected: `Build succeeded: 0 Warning(s), 0 Error(s)`.

- [ ] **Step 4: Validate all QML resources with `qmllint`**

Run:
```powershell
Get-ChildItem -Path "Monitor Matrix\qml" -Recurse -Filter "*.qml" | ForEach-Object { & "C:\Qt\6.11.1\msvc2022_64\bin\qmllint.exe" $_.FullName }
```
Expected: All QML files pass with 0 errors.

- [ ] **Step 5: Commit**

```powershell
git add -A
git commit -m "feat: complete modern QML UI overhaul and clean up legacy widget code"
```

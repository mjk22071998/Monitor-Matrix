Monitor Matrix (Screen Drop)
=============================

A lightweight Qt-based Windows utility for designing multi-monitor application placements and launching apps or URLs into specific monitor regions.

This README is intended for developers who want to build, run, or extend the project.

Key features
------------
- Visual monitor topology editor (drag, resize, snap, alignment guides).
- Configure display resolution and primary screen (Windows only) via Screen Configuration dialog.
- Create placements for applications and URLs using drag & drop onto monitors.
- Persist and restore placement layouts (.sav custom binary format with compression + simple XOR encoding).
- Launch applications or URLs positioned to monitor regions (supports full-screen, halves, quadrants and custom rectangles).

Platform & dependencies
-----------------------
- Primary target: Windows (Display configuration and window positioning use Win32 APIs).
- UI framework: Qt (widgets). Project uses Qt modules such as QtWidgets, QtConcurrent, QtGui, QtCore.
- Built as a Visual Studio project (.vcxproj). Tested with MSVC + Qt 6.x (project uses QDataStream::Qt_6_2 version macros).
- Google Chrome is required by URL launches (LaunchService looks up chrome.exe and opens URLs in a new Chrome window).

Repository layout (important source files)
-----------------------------------------
- Source.cpp             - Application entry (loads stylesheet and shows ScreenConfigDialog then MainWindow)
- styles/app.qss         - Application stylesheet (resource)
- src/views/MainWindow.* - Main UI for creating placements and running them
- src/dialog/ScreenConfigDialog.* - Dialog to inspect and modify display configuration (uses DisplayConfigService)
- src/scene/*            - Graphics scene items and logic for monitors & placements (MonitorScene, MonitorDropItem, PlacedItem, topology items)
- src/services/*         - Platform services: DisplayConfigService (Windows display enumerate/apply), MonitorService, LaunchService
- src/managers/*         - ConfigManager (load apps.json), LayoutManager (save/load .sav layout files)
- src/models/*           - Data models: AppItem, MonitorInfo, Placement, DisplayConfigTypes
- src/strategies/*       - LaunchStrategy and implementations (ExecutableStrategy, URLStrategy)
- config/apps.json       - Application list used by AppListView (sample copies exist under x64/Debug and x64/Release)

High level architecture
-----------------------
- UI (MainWindow, ScreenConfigDialog) composes graphics scenes and widgets.
- MonitorTopologyScene manages monitor layout editing (snap, connection checks, alignment guides).
- MonitorScene and MonitorDropItem handle monitor rendering and drop targets for app placements.
- PlacedItem represents an app placement (supports drag, resize, snap, and emits events for movement / custom rects).
- DisplayConfigService abstracts system display enumeration and application (Win32 helper functions used under Q_OS_WIN).
- LaunchService encapsulates launching an executable or URL and then positioning the created window via Win32 calls.
- ConfigManager loads a JSON file of apps: each app has id, name, type (application|url), path and a LaunchStrategy is attached.
- LayoutManager persists layout data using an encoded and compressed payload. The format includes a small magic/version header and a compressed payload that stores monitors and placements.

Important implementation details / gotchas
----------------------------------------
- DisplayConfigService::applyDisplays is implemented only on Windows (Q_OS_WIN). On other platforms it emits applyFailed.
- LaunchService uses Windows APIs (SetWindowPos, ShowWindow, EnumWindows, etc.) and expects Chrome to be present for URL launches.
- Layout files (.sav) are compressed and XOR-encoded with a static key string (LayoutManager::encodePayload / decodePayload).
- The UI reads apps from config/apps.json relative to the executable directory. If the file is missing or malformed, ConfigManager returns an empty list and logs warnings.
- Many classes use Qt types (QRect, QString, QVector, signals/slots). QDataStream version is set to Qt_6_2 in serialization code.
- MonitorTopologyScene enforces: no overlapping screens, and connectivity (each screen must touch another by at least a small side contact). This constraint affects resolution/size changes.

Config / data formats
---------------------
- config/apps.json (simple array or root { "apps": [...] })
  Each app object should include at least:
	{
	  "id": "unique-id",
	  "name": "Display name",
	  "type": "application" | "url",
	  "path": "C:/path/to/exe.exe" or "https://example.com"
	}

- Layout .sav files: binary file created by LayoutManager::saveToFile. It contains a small header (magic + version) and an encoded/compressed payload. Use the app's open/save UI to read/write these files.

Building
--------
Prerequisites:
- Visual Studio (2019/2022) with MSVC toolset
- Qt 6.x installed and the Qt VS Tools (or configured Qt include/lib paths)

Quick steps (Visual Studio):
1. Open Monitor Matrix.vcxproj (or create a solution and add the project) in Visual Studio.
2. Configure the project to find Qt include/lib directories and add required Qt modules to the linker (Widgets, Core, Gui, Concurrent).
3. Build (select Debug/Release as required).

Notes for qmake/CMake users:
- This code is a Visual Studio project by default; porting to qmake/CMake requires adding appropriate targets and linking Qt modules.

Running
-------
- Ensure config/apps.json is present next to the built executable (or under the expected config folder). Sample files exist in x64/Debug/config and x64/Release/config in the repo.
- Start the app. The first dialog is the screen configuration dialog where you can inspect and (on Windows) apply resolution/primary changes. After closing it the main window opens where you can drag apps onto monitors and save/load layouts.

Extending the project
---------------------
- Adding new launch strategies: implement LaunchStrategy and provide new strategy instances in ConfigManager when parsing the app config.
- Support additional placement positions: extend AppPosition enum (src/enums/AppPositionEnum.h) and update LaunchService::calculateTargetRect accordingly.
- Cross-platform improvements: abstract platform-specific DisplayConfigService and LaunchService to provide implementations for macOS/Linux. Be aware a number of Win32 APIs are used and must be reimplemented per platform.

Debugging and troubleshooting
-----------------------------
- Missing icons / file paths: ConfigManager logs warnings if the apps.json entries are malformed or files are missing.
- Display apply failures: DisplayConfigService emits applyFailed with a message; ScreenConfigDialog shows a QMessageBox on failure.
- Launch issues: LaunchService logs warnings if it cannot start a process or find a window. For URLs, Chrome must be installed and accessible.
- Serialization errors: LayoutManager checks magic/version and returns false on mismatch. Use the app UI to avoid loading incompatible files.

Code style & conventions
------------------------
- Code uses Qt naming and types (Q-prefixed classes). Signals/slots are used for inter-component communication.
- C++ files use header guards (#pragma once) and small classes grouped by responsibility.
- Binary serialization explicitly sets QDataStream version to Qt_6_2.

Contributing
------------
- Fork the repository, make changes on a feature branch, and submit a pull request.
- Include a short description, the rationale for changes, and any platform-specific considerations.

References
----------
- Qt documentation: https://doc.qt.io/
- Windows API docs for display and window management (EnumDisplaySettings, ChangeDisplaySettingsEx, SetWindowPos, EnumWindows, etc.)

Acknowledgements
----------------
- Built with Qt and Windows APIs.

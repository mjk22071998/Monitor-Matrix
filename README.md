# Monitor Matrix — Screen Drop

A lightweight Qt-based Windows utility to design and launch application placements across multiple monitors.

Summary
-------
Monitor Matrix (Screen Drop) provides a visual editor to arrange monitor topology, create application placements (full screen, halves, quadrants, or custom regions), and launch executables or URLs into those regions. The project uses Qt widgets for the UI and Win32 APIs for display configuration and window positioning.

Quick start (Windows)
----------------------
Prerequisites:
- Visual Studio (2019/2022/2024) with MSVC toolset
- Qt 6.x (matching your MSVC runtime)

Build steps:
1. Open the Visual Studio project: `Monitor Matrix/Screen Drop.vcxproj`.
2. Configure Qt include/lib paths or the Qt VS Tools kit.
3. Select a configuration (Debug/Release) and build the project.

Run:
- Place a `config/apps.json` next to the built executable (or provide your own). The app loads this file to populate the app catalog.
- Launch the built executable. The Screen Configuration dialog appears first (Windows-only features enabled), then the main window.

Repository layout (important)
----------------------------
- Monitor Matrix/Screen Drop.vcxproj — Visual Studio project
- src/ — application source (views, dialogs, scenes, services, managers, models, strategies)
- styles/app.qss — application stylesheet
- config/apps.json — example app catalog (provide your copy next to the executable)

Notes about removed files
-------------------------
During cleanup, the following non-source files were removed from the repository because they are build artifacts or editor helper files:
- x64/Debug/config/apps.json (build output)
- x64/Release/config/apps.json (build output)
- Monitor Matrix/cpp.hint (editor/helper)

If you rely on any of these files, restore them from a local build output or add your own config/apps.json next to the executable.

Important implementation details
-------------------------------
- Many platform-specific features (display apply, window placement, URL launching) are implemented with Win32 APIs and therefore work only on Windows.
- URL launching looks for Chrome (chrome.exe) when opening URLs.
- Layout files (.sav) are binary, compressed, and XOR-encoded by LayoutManager. Use the app UI to save/load layouts.

Contributing
------------
- Fork, create a feature branch, and open a pull request with a clear description.
- If you want a .gitignore, CI, or a CMake/qmake port, open an issue or submit a PR.

Support
-------
Open an issue in the repository describing the problem and steps to reproduce.

License
-------
Add a LICENSE file to indicate the project license. If none is present, assume the repo has no license specified.

Contact
-------
For questions or requested changes to this README, open an issue or send a PR.

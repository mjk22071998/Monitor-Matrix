# Modern CMake Guidelines for C++ & Qt 6

## 1. Target-Based CMake Philosophy

Modern CMake is entirely target-centric. Target properties encapsulate compilation requirements, include directories, compiler definitions, and linker dependencies.

### Hard Rules
- **NEVER use directory-level commands**:
  - Prohibited: `include_directories()`, `link_directories()`, `link_libraries()`, `add_definitions()`.
  - Mandatory: `target_include_directories()`, `target_link_libraries()`, `target_compile_definitions()`.
- **Always specify target visibility**:
  - `PRIVATE`: Required only to build this target.
  - `PUBLIC`: Required to build this target AND by anyone linking against it.
  - `INTERFACE`: Required ONLY by consumers linking against this target (e.g. header-only libraries).

---

## 2. Qt 6 Project Setup

### 2.1 Standard Initialization
Call `qt_standard_project_setup()` directly after locating Qt 6:

```cmake
cmake_minimum_required(VERSION 3.25)
project(ScreenDrop VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always specify explicit minimum Qt minor version
find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

# Automatically configures AUTOMOC, AUTOUIC, GNUInstallDirs
qt_standard_project_setup()
```

### 2.2 Target Definition with `qt_add_executable`
Use `qt_add_executable` instead of raw `add_executable` to properly handle platform entry points, app bundles, and icon embedding:

```cmake
qt_add_executable(screen_drop
    main.cpp
    src/views/MainWindow.cpp
    src/views/MainWindow.h
    src/services/LaunchService.cpp
    src/services/LaunchService.h
)

target_include_directories(screen_drop
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(screen_drop
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)
```

---

## 3. Resources & Static Assets

Use `qt_add_resources` with explicit prefixes:

```cmake
qt_add_resources(screen_drop "app_resources"
    PREFIX "/"
    FILES
        Resources/icons/action-run.svg
        Resources/icons/status-check.svg
        Resources/fonts/DMSans.ttf
)
```

For QML applications, **always use `qt_add_qml_module`** rather than raw `.qrc` files:

```cmake
qt_add_qml_module(screen_drop
    URI ScreenDrop.UI
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/MonitorView.qml
)
```

---

## 4. Compiler Warnings & Quality Gates

Enforce high diagnostic levels per target:

```cmake
if(MSVC)
    target_compile_options(screen_drop PRIVATE
        /W4
        /WX- # Warn on error toggle
        /permissive-
        /utf-8
        /Zc:__cplusplus
    )
else()
    target_compile_options(screen_drop PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wnon-virtual-dtor
        -Wold-style-cast
    )
endif()
```

---

## 5. CMake Presets (`CMakePresets.json`)

Standardize build configurations using `CMakePresets.json` (version 6+):
- `default`: Base configuration with Ninja generator.
- `debug`: Debug build with AddressSanitizer and debug symbols.
- `release`: Optimized release build.
- `test`: Automated test runner setup for CTest.

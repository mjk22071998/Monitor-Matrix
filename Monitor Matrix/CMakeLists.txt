# Standalone project support if opened or configured directly from this folder
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.20)

    project(ScreenDrop
        VERSION 1.0.0
        DESCRIPTION "Screen Drop - Multi-Monitor Application Placement & Display Management"
        LANGUAGES CXX
    )

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    include(CTest)

    find_package(Qt6 6.8 REQUIRED COMPONENTS
        Core
        Gui
        Quick
        QuickControls2
        Widgets
        Test
    )

    qt_standard_project_setup(REQUIRES 6.8)
endif()

# ==============================================================================
# 1. Core Static Library: screendrop_core
# ==============================================================================
qt_add_library(screendrop_core STATIC
    # Controllers
    src/controllers/DisplayConfigBridge.cpp
    src/controllers/DisplayConfigBridge.h
    src/controllers/ScreenDropEngine.cpp
    src/controllers/ScreenDropEngine.h

    # Enums
    src/enums/AppPosition.h
    src/enums/AppPositionEnum.h

    # Managers
    src/managers/ConfigManager.cpp
    src/managers/ConfigManager.h
    src/managers/LayoutManager.cpp
    src/managers/LayoutManager.h

    # Models
    src/models/AppItem.h
    src/models/DisplayConfig.h
    src/models/DisplayConfigTypes.h
    src/models/MonitorInfo.h
    src/models/Placement.h
    src/models/qml/AppCatalogModel.cpp
    src/models/qml/AppCatalogModel.h
    src/models/qml/MonitorCanvasModel.cpp
    src/models/qml/MonitorCanvasModel.h
    src/models/qml/PlacementModel.cpp
    src/models/qml/PlacementModel.h

    # Platform (Windows)
    src/platform/windows/WindowsDisplayModes.cpp
    src/platform/windows/WindowsDisplayModes.h
    src/platform/windows/WindowsDisplayTopology.cpp
    src/platform/windows/WindowsDisplayTopology.h
    src/platform/windows/WindowsShortcutResolver.cpp
    src/platform/windows/WindowsShortcutResolver.h
    src/platform/windows/WindowsTaskRunner.cpp
    src/platform/windows/WindowsTaskRunner.h

    # Services
    src/services/AppCatalogService.cpp
    src/services/AppCatalogService.h
    src/services/DisplayConfigService.cpp
    src/services/DisplayConfigService.h
    src/services/LaunchProcess.cpp
    src/services/LaunchProcess.h
    src/services/LaunchService.cpp
    src/services/LaunchService.h
    src/services/LaunchWindowPlacement.cpp
    src/services/LaunchWindowPlacement.h
    src/services/LayoutLaunchService.cpp
    src/services/LayoutLaunchService.h
    src/services/MonitorService.cpp
    src/services/MonitorService.h

    # Strategies
    src/strategies/ExecutableStrategy.cpp
    src/strategies/ExecutableStrategy.h
    src/strategies/LaunchStrategy.cpp
    src/strategies/LaunchStrategy.h
    src/strategies/URLStrategy.cpp
    src/strategies/URLStrategy.h

    # Theme
    src/theme/AppTheme.cpp
    src/theme/AppTheme.h

    # Utils
    src/utils/AppLogger.cpp
    src/utils/AppLogger.h
)

target_include_directories(screendrop_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(screendrop_core
    PUBLIC
        UNICODE
        _UNICODE
        NOMINMAX
)

if(MSVC)
    target_compile_options(screendrop_core
        PRIVATE
            /FS
            /Zc:__cplusplus
            /permissive-
    )
endif()

target_link_libraries(screendrop_core
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
)

if(WIN32)
    target_link_libraries(screendrop_core
        PRIVATE
            user32
            gdi32
            shell32
            ole32
            oleaut32
            advapi32
    )
endif()

# Embedded Assets: Icons and Fonts
qt_add_resources(screendrop_core "screendrop_icons"
    PREFIX "/icons"
    BASE "Resources/icons"
    FILES
        Resources/icons/action-run.svg
        Resources/icons/action-add.svg
        Resources/icons/action-remove.svg
        Resources/icons/status-warning.svg
        Resources/icons/status-check.svg
)

qt_add_resources(screendrop_core "screendrop_fonts"
    PREFIX "/fonts"
    BASE "Resources/fonts"
    FILES
        Resources/fonts/DMSans.ttf
        Resources/fonts/DMSans-Italic.ttf
)

# ==============================================================================
# 2. Main Application Executable: ScreenDrop ("Screen Drop.exe")
# ==============================================================================
qt_add_executable(ScreenDrop WIN32
    main.cpp
)

set_target_properties(ScreenDrop PROPERTIES
    OUTPUT_NAME "Screen Drop"
    WIN32_EXECUTABLE TRUE
)

# Configure singleton type property BEFORE qt_add_qml_module
set_source_files_properties(qml/theme/Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(ScreenDrop
    URI ScreenDrop
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        qml/Main.qml
        qml/theme/Theme.qml
        qml/components/HeaderBar.qml
        qml/components/OverlapStatusPill.qml
        qml/components/NotificationToast.qml
        qml/components/PlacedAppItem.qml
        qml/components/MonitorItem.qml
        qml/components/CanvasControls.qml
        qml/views/AppSidebar.qml
        qml/views/CanvasView.qml
        qml/dialogs/AppEditModal.qml
        qml/dialogs/ScreenConfigSheet.qml
)

target_link_libraries(ScreenDrop
    PRIVATE
        screendrop_core
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
)

# ==============================================================================
# 3. Automated Post-Build Deployment (windeployqt)
# ==============================================================================
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS "${_qt6Core_install_prefix}/bin"
    )
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ScreenDrop POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                    --no-compiler-runtime
                    "$<TARGET_FILE:ScreenDrop>"
            COMMENT "Deploying Qt runtime dependencies using windeployqt..."
        )
    endif()
endif()

# Standard Install & Packaging Deployment Script
install(TARGETS ScreenDrop screendrop_core
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET ScreenDrop
    OUTPUT_SCRIPT deploy_script
    DEPLOY_TOOL_OPTIONS "--no-compiler-runtime"
)
install(SCRIPT ${deploy_script})

# ==============================================================================
# 4. Unit Test Targets (CTest)
# ==============================================================================
if(BUILD_TESTING)
    # SanityTest
    qt_add_executable(sanity_test
        src/tests/SanityTest.cpp
    )
    target_link_libraries(sanity_test
        PRIVATE
            Qt6::Core
            Qt6::Quick
            Qt6::Test
    )
    add_test(NAME SanityTest COMMAND sanity_test)

    # ModelUnitTests
    qt_add_executable(model_unit_tests
        src/tests/ModelUnitTests.cpp
    )
    target_link_libraries(model_unit_tests
        PRIVATE
            screendrop_core
            Qt6::Core
            Qt6::Gui
            Qt6::Test
    )
    add_test(NAME ModelUnitTests COMMAND model_unit_tests)

    # EngineTests
    qt_add_executable(engine_tests
        src/tests/EngineTests.cpp
    )
    target_link_libraries(engine_tests
        PRIVATE
            screendrop_core
            Qt6::Core
            Qt6::Gui
            Qt6::Test
    )
    add_test(NAME EngineTests COMMAND engine_tests)
endif()

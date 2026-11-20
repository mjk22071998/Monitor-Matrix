cmake_minimum_required(VERSION 3.25)
project(ModernQtApp VERSION 1.0.0 LANGUAGES CXX)

# Enforce Modern C++23 Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for Clang-Tidy and LSP servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Qt 6 with explicit minimum version
find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

# Standard Qt project setup (Automoc, Autouic, GNUInstallDirs)
qt_standard_project_setup()

# Application Target
qt_add_executable(modern_qt_app
    src/main.cpp
)

target_include_directories(modern_qt_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(modern_qt_app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Target-specific compiler diagnostics
if(MSVC)
    target_compile_options(modern_qt_app PRIVATE
        /W4
        /permissive-
        /utf-8
        /Zc:__cplusplus
    )
else()
    target_compile_options(modern_qt_app PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wnon-virtual-dtor
    )
endif()

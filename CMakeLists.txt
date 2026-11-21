cmake_minimum_required(VERSION 3.20)

project(ScreenDrop
    VERSION 1.0.0
    DESCRIPTION "Screen Drop - Multi-Monitor Application Placement & Display Management"
    LANGUAGES CXX
)

# Enforce Modern C++ Standard (C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable CTest integration for automated unit testing
include(CTest)

# Find Qt 6 packages with explicit minimum version pin
find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Widgets
    Test
)

# Modern Qt Standard Project Setup
# Automatically configures AUTOMOC, AUTOUIC, GNUInstallDirs, and modern CMake policies (e.g. QTP0001, QTP0004, QTP0005)
qt_standard_project_setup(REQUIRES 6.8)

# Add application subdirectory
add_subdirectory("Monitor Matrix")

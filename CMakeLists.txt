cmake_minimum_required(VERSION 3.21.1)

# NOTE: the version setting is parsed by flake.nix, so be careful with changes.
set(version "0.0.1")

project(helloworld VERSION "${version}" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(helloworld
    # src/mainwindow.ui
    # src/mainwindow.cpp
    src/main.cpp
)

target_link_libraries(helloworld PRIVATE Qt6::Widgets)

set_target_properties(helloworld PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

install(TARGETS helloworld)

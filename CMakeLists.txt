cmake_minimum_required(VERSION 3.21.1)

# NOTE: the version setting is parsed by flake.nix, so be careful with changes.
set(version "0.0.1")

project(git-monitor VERSION "${version}" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGIT2 REQUIRED IMPORTED_TARGET libgit2)

find_package(fmt REQUIRED)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

qt_add_executable(git-monitor
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/addrepodialog.cpp
    src/addrepodialog.h
    src/addrepodialog.ui
    src/git/branch_iterator.cpp
    src/git/branch_iterator.h
    src/git/oid.cpp
    src/git/oid.h
    src/git/reference.cpp
    src/git/reference.h
    src/git/remote.cpp
    src/git/remote.h
    src/git/repository.cpp
    src/git/repository.h
    src/git/util.cpp
    src/git/util.h
)

target_link_libraries(git-monitor
    PRIVATE
        Qt::Core
        Qt::Widgets
        PkgConfig::LIBGIT2
        fmt::fmt
)

include(GNUInstallDirs)

install(TARGETS git-monitor
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET git-monitor
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

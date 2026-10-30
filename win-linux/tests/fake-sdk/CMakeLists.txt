# SPDX-FileCopyrightText: 2026 Euro-Office contributors
# SPDX-License-Identifier: AGPL-3.0-only

# Unit test build: the application code (desktopeditors.cmake) against a fake
# desktop-sdk instead of the real one, so it builds with Qt, GoogleTest and
# the platform libraries alone, without core, CEF or the docker build. Only
# the desktop-sdk headers and a few header-only files from core are needed;
# core's CFileDownloader, which the app still uses directly, is faked too.
# See ../README.md.
#
#   cmake -S win-linux/tests/fake-sdk -B build [-DEO_COVERAGE=ON]
#   cmake --build build && ctest --test-dir build --output-on-failure

cmake_minimum_required(VERSION 3.21)
project(DesktopEditorsUnitTests LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

get_filename_component(WIN_LINUX_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
# Checked out next to desktop-apps, as in the DesktopEditors superproject
set(DESKTOP_SDK_ROOT "${WIN_LINUX_DIR}/../../desktop-sdk" CACHE PATH "desktop-sdk checkout (headers only)")
set(CORE_ROOT_DIR "${WIN_LINUX_DIR}/../../core" CACHE PATH "core checkout (header-only files only)")

option(EO_COVERAGE "Measure test coverage of the application code (gcov)" OFF)

set(QT_VERSION_MAJOR 6)
find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets PrintSupport Svg Multimedia MultimediaWidgets OpenGLWidgets Network DBus)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)

# What core's common.cmake and ../../CMakeLists.txt define for the app on Linux
add_definitions(-D_LINUX -DLINUX -D_REENTRANT -DUNICODE -D_UNICODE -D_WAYLAND
    -D__DONT_WRITE_IN_APP_TITLE -DAPP_ICON_PATH="./res/icons/desktopeditors-eo.ico"
    -DCOPYRIGHT_YEAR=2026)

include(${WIN_LINUX_DIR}/desktopeditors.cmake)
list(REMOVE_ITEM COMMON_SOURCES ${WIN_LINUX_DIR}/src/main.cpp)
qt6_add_resources(RESOURCES ${WIN_LINUX_DIR}/resources.qrc)

set(SDK_INCLUDE_DIRS
    ${DESKTOP_SDK_ROOT}/ChromiumBasedEditors/lib/include
    ${DESKTOP_SDK_ROOT}/ChromiumBasedEditors/lib/qt_wrapper/include)

file(GLOB FAKE_SDK_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.h)
# The real headers with Q_OBJECT, so moc generates their meta-object code
add_library(fake_sdk STATIC ${FAKE_SDK_SOURCES}
    ${DESKTOP_SDK_ROOT}/ChromiumBasedEditors/lib/qt_wrapper/include/qcefview.h
    ${DESKTOP_SDK_ROOT}/ChromiumBasedEditors/lib/qt_wrapper/include/qcefview_media.h)
target_include_directories(fake_sdk PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${SDK_INCLUDE_DIRS})
target_include_directories(fake_sdk PRIVATE ${CORE_ROOT_DIR}/DesktopEditor ${CORE_ROOT_DIR}/Common)
target_link_libraries(fake_sdk PUBLIC Qt6::Widgets Qt6::OpenGLWidgets Qt6::PrintSupport Qt6::MultimediaWidgets)

# Same name as in the full build, so tests/CMakeLists.txt works in both
add_library(desktopeditors_core OBJECT ${COMMON_SOURCES} ${COMMON_HEADERS} ${RESOURCES})
target_include_directories(desktopeditors_core PUBLIC
    ${WIN_LINUX_DIR}/src/prop
    ${WIN_LINUX_DIR}/src
    ${GTK3_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${SDK_INCLUDE_DIRS}
    ${CORE_ROOT_DIR}/DesktopEditor
    ${CORE_ROOT_DIR}/Common
    ${WIN_LINUX_DIR}/extras/update-daemon/src/classes)
target_link_libraries(desktopeditors_core PUBLIC
    fake_sdk
    Qt6::Widgets Qt6::Svg Qt6::PrintSupport Qt6::Multimedia Qt6::MultimediaWidgets
    Qt6::OpenGLWidgets Qt6::Network Qt6::DBus
    ${PLATFORM_LIBS} ${DBUS_LIBRARIES} X11 dl pthread)
if(EO_COVERAGE)
    target_compile_options(desktopeditors_core PRIVATE --coverage -O0 -g)
    target_link_options(desktopeditors_core PUBLIC --coverage)
endif()

# desktop_test() for tests/CMakeLists.txt: every suite but those only for the
# full build (SCOPE full), linked to the fake SDK through desktopeditors_core
function(desktop_test)
    cmake_parse_arguments(T "GTEST_MAIN" "NAME;SCOPE" "SOURCES;LIBS" ${ARGN})
    if(T_SCOPE STREQUAL "full")
        return()
    endif()
    add_executable(${T_NAME} ${T_SOURCES})
    target_link_libraries(${T_NAME} PRIVATE GTest::gtest ${T_LIBS})
    if(T_GTEST_MAIN)
        target_link_libraries(${T_NAME} PRIVATE GTest::gtest_main)
    endif()
    add_test(NAME ${T_NAME} COMMAND ${T_NAME})
endfunction()

enable_testing()
add_subdirectory(${WIN_LINUX_DIR}/tests tests)

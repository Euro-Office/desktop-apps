# SPDX-FileCopyrightText: 2026 Euro-Office contributors
# SPDX-License-Identifier: AGPL-3.0-only

# desktop-apps unit tests, registered with desktop_test() for both builds
# (see README.md):
#
# - the full build, reached from ../CMakeLists.txt when EO_BUILD_TESTS is ON,
#   links the real desktop-sdk; desktop_test() is defined below on top of
#   add_core_gtest() from core/common.cmake.
# - the unit test build in fake-sdk/ links a fake desktop-sdk and defines
#   desktop_test() itself.
#
# desktop_test(NAME <name> SOURCES <files...> [LIBS <libs...>] [GTEST_MAIN]
#              [SCOPE unit|full])
# SCOPE limits a suite to one build: "unit" for suites that drive the fake
# SDK, "full" for suites that need the real SDK. Without it, both run it.

# Shared entry point for suites that need a QApplication: it starts Qt on the
# offscreen platform, so widget code runs headless in CI.
set(DESKTOP_TEST_MAIN "${CMAKE_CURRENT_LIST_DIR}/qt_test_main.cpp")

if(NOT COMMAND desktop_test)
    # The suites run from the build tree, where Qt isn't copied next to them as
    # it is for the installed app. common.cmake sets
    # CMAKE_BUILD_WITH_INSTALL_RPATH, which gives build-tree binaries only the
    # install RPATH ($ORIGIN); turn that off so CMake adds the directories of
    # linked libraries, Qt's included.
    function(desktop_test)
        cmake_parse_arguments(T "GTEST_MAIN" "NAME;SCOPE" "SOURCES;LIBS" ${ARGN})
        if(T_SCOPE STREQUAL "unit")
            return()
        endif()
        if(T_GTEST_MAIN)
            set(_gtest_main GTEST_MAIN)
        endif()
        add_core_gtest(NAME ${T_NAME} SOURCES ${T_SOURCES} LIBS ${T_LIBS} ${_gtest_main})
        set_target_properties(${T_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH OFF)
    endfunction()
endif()

desktop_test(
    NAME    desktopeditors_setup_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/setup_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    Qt${QT_VERSION_MAJOR}::Widgets
)

desktop_test(
    NAME    desktopeditors_utils_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/utils_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    desktopeditors_core
)

desktop_test(
    NAME    desktopeditors_providers_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/providers_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    desktopeditors_core
)

desktop_test(
    NAME    desktopeditors_tabdata_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/tabdata_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    desktopeditors_core
)

desktop_test(
    NAME    desktopeditors_tabpanel_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/tabpanel_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    desktopeditors_core
    SCOPE   unit
)

# SPDX-FileCopyrightText: 2026 Euro-Office contributors
# SPDX-License-Identifier: AGPL-3.0-only

# desktop-apps unit tests. Reached from ../CMakeLists.txt when EO_BUILD_TESTS
# is ON; every suite is a GoogleTest executable registered with CTest through
# add_core_gtest() from core/common.cmake. See README.md.

# Shared entry point for suites that need a QApplication: it starts Qt on the
# offscreen platform, so widget code runs headless in CI.
set(DESKTOP_TEST_MAIN "${CMAKE_CURRENT_LIST_DIR}/qt_test_main.cpp")

# The suites run from the build tree, where Qt isn't copied next to them as it
# is for the installed app. common.cmake sets CMAKE_BUILD_WITH_INSTALL_RPATH,
# which gives build-tree binaries only the install RPATH ($ORIGIN); turn that
# off so CMake adds the directories of linked libraries, Qt's included.
function(use_build_tree_qt target)
    set_target_properties(${target} PROPERTIES BUILD_WITH_INSTALL_RPATH OFF)
endfunction()

add_core_gtest(
    NAME    desktopeditors_setup_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/setup_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    Qt${QT_VERSION_MAJOR}::Widgets
)
use_build_tree_qt(desktopeditors_setup_test)

add_core_gtest(
    NAME    desktopeditors_utils_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/utils_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    desktopeditors_core
)
use_build_tree_qt(desktopeditors_utils_test)

add_core_gtest(
    NAME    desktopeditors_providers_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/providers_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    desktopeditors_core
)
use_build_tree_qt(desktopeditors_providers_test)

add_core_gtest(
    NAME    desktopeditors_tabdata_test
    SOURCES ${CMAKE_CURRENT_LIST_DIR}/tabdata_test.cpp ${DESKTOP_TEST_MAIN}
    LIBS    desktopeditors_core
)
use_build_tree_qt(desktopeditors_tabdata_test)

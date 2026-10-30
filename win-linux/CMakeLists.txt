cmake_minimum_required(VERSION 3.21)


# 1. Project Setup
project(DesktopEditors VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CORE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../core")
set(DESKTOP_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../desktop-sdk")

set(BUILD_DESKTOP TRUE CACHE INTERNAL "tell common to prepare third party for desktop")
include(${CORE_ROOT_DIR}/common.cmake)

# Unit tests, see tests/README.md. Needs the vcpkg 'tests' manifest feature.
option(EO_BUILD_TESTS "Build the desktop-apps and desktop-sdk unit tests" OFF)
if(EO_BUILD_TESTS)
    # Before the desktop-sdk subdirectories below, so their tests are
    # registered with this build tree's CTest as well.
    enable_testing()
endif()


if(UNIX)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
    add_definitions(-DLINUX -D_LINUX -D_WAYLAND)
endif()

if(NOT TARGET hunspell-wrapper)
    add_subdirectory(${CORE_ROOT_DIR}/Common/3dParty/hunspell/qt hunspell-wrapper)
endif()

if(NOT TARGET PdfFile)
    add_subdirectory(${CORE_ROOT_DIR}/PdfFile PdfFile)
endif()

if(NOT TARGET XpsFile)
    add_subdirectory(${CORE_ROOT_DIR}/XpsFile XpsFile)
endif()

if(NOT TARGET DjVuFile)
    add_subdirectory(${CORE_ROOT_DIR}/DjVuFile DjVuFile)
endif()

if(NOT TARGET ooxmlsignature)
    add_subdirectory(${CORE_ROOT_DIR}/DesktopEditor/xmlsec/src ooxmlsignature)
endif()

if(NOT TARGET kernel)
    add_subdirectory(${CORE_ROOT_DIR}/Common kernel)
endif()

if(NOT TARGET kernel_network)
    add_subdirectory(${CORE_ROOT_DIR}/Common/Network kernel_network)
endif()

if(NOT TARGET UnicodeConverter)
    add_subdirectory(${CORE_ROOT_DIR}/UnicodeConverter UnicodeConverter)
endif()

if(NOT TARGET graphics)
    add_subdirectory(${CORE_ROOT_DIR}/DesktopEditor/graphics/cmake graphics)
endif()

if(NOT TARGET ascdocumentscore)
    add_subdirectory(${DESKTOP_SDK_ROOT}/ChromiumBasedEditors/lib/ ascdocumentscore)
endif()

if(NOT TARGET qtascdocumentscore)
    add_subdirectory(${DESKTOP_SDK_ROOT}/ChromiumBasedEditors/lib/qt_wrapper/ qtascdocumentscore)
endif()

if(NOT TARGET doctrenderer)
    add_subdirectory(${CORE_ROOT_DIR}/DesktopEditor/doctrenderer doctrenderer)
endif()

if(NOT TARGET DocxRenderer)
    add_subdirectory(${CORE_ROOT_DIR}/DocxRenderer DocxRenderer)
endif()

if(NOT TARGET Fb2File)
    add_subdirectory(${CORE_ROOT_DIR}/Fb2File Fb2File)
endif()

if(NOT TARGET EpubFile)
    add_subdirectory(${CORE_ROOT_DIR}/EpubFile EpubFile)
endif()

if(NOT TARGET HtmlFile2)
    add_subdirectory(${CORE_ROOT_DIR}/HtmlFile2 HtmlFile2)
endif()

if(NOT TARGET OFDFile)
    add_subdirectory(${CORE_ROOT_DIR}/OFDFile OFDFile)
endif()

if(NOT TARGET IWorkFile)
    add_subdirectory(${CORE_ROOT_DIR}/Apple IWorkFile)
endif()

if(NOT TARGET HWPFile)
    add_subdirectory(${CORE_ROOT_DIR}/HwpFile HWPFile)
endif()

if(NOT TARGET x2t)
    add_subdirectory( "${CORE_ROOT_DIR}/X2tConverter/build/cmake" x2t )
endif()

if(NOT TARGET allfontsgen)
    add_subdirectory( "${CORE_ROOT_DIR}/DesktopEditor/AllFontsGen" allfontsgen )
endif()

if(NOT TARGET allthemesgen)
    add_subdirectory( "${CORE_ROOT_DIR}/DesktopEditor/allthemesgen" allthemesgen )
endif()



# 3. Definitions & Global Config
add_definitions(-D__DONT_WRITE_IN_APP_TITLE)
add_definitions(-DAPP_ICON_PATH="./res/icons/desktopeditors-eo.ico")
add_definitions(-DCOPYRIGHT_YEAR=2026)

# Handle Product Version from Environment
set(ENV_PRODUCT_VERSION $ENV{PRODUCT_VERSION})
if(ENV_PRODUCT_VERSION)
    set(FULL_PRODUCT_VERSION "${ENV_PRODUCT_VERSION}.$ENV{BUILD_NUMBER}")
    add_definitions(-DVER_PRODUCT_VERSION=${FULL_PRODUCT_VERSION})
    # VERSIONINFO FILEVERSION needs exactly four numbers, so drop suffixes
    # like ".dev" (9.3.5.dev + build 1 -> 9,3,5,1)
    string(REGEX MATCHALL "[0-9]+" _ver_nums "${ENV_PRODUCT_VERSION}")
    list(APPEND _ver_nums 0 0 0)
    list(SUBLIST _ver_nums 0 3 _ver_nums)
    set(_ver_build "$ENV{BUILD_NUMBER}")
    if(NOT _ver_build MATCHES "^[0-9]+$")
        set(_ver_build 0)
    endif()
    list(APPEND _ver_nums ${_ver_build})
    string(REPLACE ";" "," VERSION_COMMAS "${_ver_nums}")
    add_definitions(-DVER_PRODUCT_VERSION_COMMAS=${VERSION_COMMAS})
endif()

# 4.-6. Sources, headers and platform libraries, see desktopeditors.cmake
include(${CMAKE_CURRENT_LIST_DIR}/desktopeditors.cmake)
# core code the app compiles in itself, kept out of the unit test build
list(APPEND COMMON_SOURCES ${CORE_ROOT_DIR}/Common/3dParty/openssl/common/common_openssl.cpp)


# 7. Create Library and Executable
qt6_add_resources(RESOURCES resources.qrc)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extras.qrc")
    qt6_add_resources(RESOURCES extras.qrc)
endif()

# Everything but main() is compiled once into an object library, so the unit
# tests in tests/ can link the application code. The executable takes its
# objects directly and gets its dependencies applied itself (section 8), not
# through linking desktopeditors_core: through the library's link interface
# CMake orders them differently, and the app then binds OpenSSL to the copy
# that libkernel_network/libooxmlsignature export instead of linking it
# statically. This way the executable links exactly as when it compiled the
# sources itself.
list(REMOVE_ITEM COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
add_library(desktopeditors_core OBJECT ${COMMON_SOURCES} ${COMMON_HEADERS} ${RESOURCES})
set_default_options(desktopeditors_core)

add_executable(DesktopEditors src/main.cpp $<TARGET_OBJECTS:desktopeditors_core>)

set_default_options(DesktopEditors)

if(WIN32)
    # On Windows the editor ships as editors.exe; DesktopEditors.exe is the
    # projicons launcher below, which the installer, shortcuts and file
    # associations call (jump list, --assoc, file type icons).
    set_target_properties(DesktopEditors PROPERTIES WIN32_EXECUTABLE TRUE OUTPUT_NAME editors)
    target_link_libraries(DesktopEditors PRIVATE Qt6::EntryPointPrivate)

    # processorArchitecture of the "converter" side-by-side assembly
    # (converter\converter.manifest) and of everything depending on it
    string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" _msvc_arch)
    if(_msvc_arch STREQUAL "arm64")
        set(EO_MANIFEST_ARCH arm64)
    elseif(_msvc_arch STREQUAL "x86")
        set(EO_MANIFEST_ARCH x86)
    else()
        set(EO_MANIFEST_ARCH amd64)
    endif()

    # The whole manifest of editors.exe lives in DesktopEditors.exe.manifest;
    # the linker only adds an empty one for mt.exe to merge it into.
    configure_file(res/manifest/DesktopEditors.exe.manifest
        ${CMAKE_CURRENT_BINARY_DIR}/editors.exe.manifest @ONLY)

    # App icon (IDI_MAINICON) + version info
    target_sources(DesktopEditors PRIVATE
        version.rc
        ${CMAKE_CURRENT_BINARY_DIR}/editors.exe.manifest
    )
    set_source_files_properties(version.rc PROPERTIES
        INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_options(DesktopEditors PRIVATE /MANIFESTUAC:NO)
endif()

if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DBUS REQUIRED dbus-1)
endif()

if(ABOUT_PAGE_APP_NAME)
    add_compile_definitions(ABOUT_PAGE_APP_NAME="${ABOUT_PAGE_APP_NAME}")
endif()

if(DESKTOP_FILE_NAME)
    add_compile_definitions(DESKTOP_FILE_NAME="${DESKTOP_FILE_NAME}")
endif()

# 8. Include Paths and Dependencies
# Applied to the executable (PRIVATE) and to desktopeditors_core (PUBLIC, so
# the unit tests get the same include paths, definitions and libraries as the
# application code). See section 7 for why the executable gets its own copy.
function(desktopeditors_add_dependencies target scope)
    target_include_directories(${target} ${scope}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/prop
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${GTK3_INCLUDE_DIRS}
        ${DBUS_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../desktop-sdk/ChromiumBasedEditors/lib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../desktop-sdk/ChromiumBasedEditors/lib/qt_wrapper/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../core/DesktopEditor
        ${CMAKE_CURRENT_SOURCE_DIR}/../../core/Common
        ${CMAKE_CURRENT_SOURCE_DIR}/extras/update-daemon/src/classes
        ${OPENSSL_INSTALL_DIR_ABS}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../core/Common/3dParty/openssl/common
        ${CORE_ROOT_DIR}/Common/3dParty/openssl/common
    )

    if(WIN32)
        target_link_libraries(${target} ${scope}
            Qt6::CorePrivate
            Qt6::GuiPrivate
            Qt6::PrintSupportPrivate)
    endif()

    target_link_directories(${target} ${scope}
        $<$<CONFIG:Debug>:${CEF_ROOT}/Debug>
        $<$<NOT:$<CONFIG:Debug>>:${CEF_ROOT}/Release>
        ${ICU_INSTALL_DIR}/include
    )

    target_link_libraries(${target} ${scope}
        # --- UI and High Level ---
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Svg
        Qt${QT_VERSION_MAJOR}::PrintSupport
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::MultimediaWidgets
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        ${PLATFORM_LIBS}
        ${DBUS_LIBRARIES}

        # --- Circular Dependency Group ---
        # This forces the linker to resolve symbols between core libs and dependencies
        -Wl,--start-group
            ascdocumentscore
            qtascdocumentscore
            kernel
            kernel_network
            graphics
            UnicodeConverter
            PdfFile
            DjVuFile
            XpsFile
            ${OPENSSL_LIBSSL}
            ${OPENSSL_LIBCRYPTO}
        -Wl,--end-group

        # --- System Essentials ---
        hunspell_wrapper
        ooxmlsignature
    )

    if(WIN32)
        target_link_libraries(${target} ${scope} libcef)
    else()
        target_link_libraries(${target} ${scope} cef X11 dl pthread rt z)
    endif()

    target_compile_definitions(${target} ${scope} COMMON_OPENSSL_BUILDING_INTERNAL)

    if(WIN32)
        target_compile_definitions(${target} ${scope} UNICODE _UNICODE WIN32 _WINDOWS _WIN32)
    endif()

    if(UNIX)
        target_link_libraries(${target} ${scope}
            Qt${QT_VERSION_MAJOR}::DBus
            ${LIBICUUC}
            ${LIBICUDATA}
            ${LIBICUI}
        )
    endif()
endfunction()

desktopeditors_add_dependencies(DesktopEditors PRIVATE)
desktopeditors_add_dependencies(desktopeditors_core PUBLIC)

if(UNIX)
    target_link_options(DesktopEditors PRIVATE "-Wl,-rpath,$ORIGIN" "-Wl,-rpath,$ORIGIN/converter")
endif()

if(WIN32)
    # editors.exe declares its dependencies in DesktopEditors.exe.manifest
    foreach(_t editors_helper
            doctrenderer DocxRenderer kernel kernel_network graphics
            UnicodeConverter PdfFile DjVuFile XpsFile ooxmlsignature
            Fb2File EpubFile HtmlFile2 OFDFile IWorkFile HWPFile
            StarMathConverter)
        target_link_options(${_t} PRIVATE
            "/MANIFESTDEPENDENCY:type='win32' name='converter' version='1.0.0.0' processorArchitecture='${EO_MANIFEST_ARCH}'")
    endforeach()
endif()

# 9. Windows launcher (extras/projicons -> DesktopEditors.exe)
# Handles --create-jump-list / --remove-jump-list / --assoc from the installer
# and the app, carries the file type icons referenced by index from the
# registry and shortcuts, and otherwise starts editors.exe.
if(WIN32)
    set(PROJICONS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extras/projicons)
    add_executable(projicons WIN32
        ${PROJICONS_DIR}/src/main.cpp
        ${PROJICONS_DIR}/version.rc
    )
    if(NOT BUILD_XP)
        target_sources(projicons PRIVATE
            ${PROJICONS_DIR}/src/jumplist.cpp
            ${PROJICONS_DIR}/src/shellassoc.cpp
        )
    endif()
    set_target_properties(projicons PROPERTIES OUTPUT_NAME DesktopEditors)
    # projicons dir first for its res/icons/*.ico; this dir for APP_ICON_PATH
    target_include_directories(projicons PRIVATE
        ${PROJICONS_DIR}/src
        ${PROJICONS_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_definitions(projicons PRIVATE
        UNICODE _UNICODE
        APP_USER_MODEL_ID="ASC.Documents.5"
    )
    target_link_libraries(projicons PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt6::EntryPointPrivate
        shlwapi shell32 advapi32 crypt32 rpcrt4 ole32
    )
    # version.rc embeds its own manifest
    target_link_options(projicons PRIVATE /MANIFEST:NO)
endif()

# 10. Translations
file(GLOB TS_FILES "langs/*.ts")
qt6_add_translation(DesktopEditors TS_FILES ${TS_FILES})

# 1. Explicitly set CEF Path
set(CEF_LIB_PATH "${CEF_ROOT}/Release")
set(CEF_RESOURCES_PATH "${CEF_ROOT}/Resources")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/desktopeditors" CACHE PATH "Default install path" FORCE)
set(CMAKE_INSTALL_BINDIR ".")

# 2. Main Target Installation  (RUNTIME = .exe/.dll on Windows, LIBRARY = .so on Linux)
install(TARGETS
    editors_helper
    DesktopEditors
    ascdocumentscore
    qtascdocumentscore
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
    install(TARGETS projicons RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    # Start menu / desktop shortcuts and the uninstall entry use {app}\app.ico
    install(FILES res/icons/desktopeditors-eo.ico
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        RENAME app.ico
    )
endif()

# CEF binaries
if(WIN32)
    install(DIRECTORY "${CEF_LIB_PATH}/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING
        PATTERN "*.dll"
        PATTERN "*.bin"
        PATTERN "*.json"
    )
else()
    install(DIRECTORY "${CEF_LIB_PATH}/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING
        PATTERN "*.so"
        PATTERN "*.so.*"
        PATTERN "*.bin"
        PATTERN "*.json"
    )
endif()

# CEF Resources (pak files, locales, icudtl.dat) — identical layout on both platforms
install(DIRECTORY "${CEF_RESOURCES_PATH}/"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ICU runtime for the main app
if(WIN32)
    install(DIRECTORY "${ICU_INSTALL_DIR_ABS}/lib/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "icu*.dll"
    )
else()
    install(DIRECTORY "${ICU_INSTALL_DIR_ABS}/lib/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "lib*.so*"
    )
endif()

# Qt runtime
if(WIN32)
    install(DIRECTORY "${QT_ROOT}/bin/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "Qt6*.dll"
    )
    install(DIRECTORY "${QT_ROOT}/plugins/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "*.dll"
    )
else()
    install(DIRECTORY "${QT_ROOT}/lib/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "lib*.so*"
    )
    install(DIRECTORY "${QT_ROOT}/plugins/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# vcpkg runtime DLLs (hunspell, etc. — x64-windows triplet is dynamic)
if(WIN32)
    install(DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "*.dll"
    )
endif()

# Core dependencies → converter/
install(DIRECTORY "${EO_CORE_OUTPUT_DIR}/"
    USE_SOURCE_PERMISSIONS
    DESTINATION ${CMAKE_INSTALL_BINDIR}/converter
)

# Makes converter\ the private "converter" assembly the binaries above depend
# on; without it they fail with 0xC0150002 (STATUS_SXS_CANT_GEN_ACTCTX)
if(WIN32)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CORE_ROOT_DIR}/Common/msvc/converter.manifest")
    file(READ "${CORE_ROOT_DIR}/Common/msvc/converter.manifest" _converter_manifest)
    string(REPLACE "processorArchitecture=\"amd64\""
        "processorArchitecture=\"${EO_MANIFEST_ARCH}\""
        _converter_manifest "${_converter_manifest}")
    if(NOT _converter_manifest MATCHES "processorArchitecture=\"${EO_MANIFEST_ARCH}\"")
        message(FATAL_ERROR "converter.manifest: could not set processorArchitecture to ${EO_MANIFEST_ARCH}")
    endif()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/converter.manifest" "${_converter_manifest}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/converter.manifest"
        DESTINATION ${CMAKE_INSTALL_BINDIR}/converter
    )
endif()

# Boost: on Windows it's static (linked into the binaries) — nothing to install
if(NOT WIN32)
    install(DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/boost/linux_64/lib/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}/converter
        FILES_MATCHING PATTERN "lib*.so*"
    )
endif()

if(EO_BUILD_TESTS)
    add_subdirectory(tests)
endif()

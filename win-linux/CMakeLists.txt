cmake_minimum_required(VERSION 3.16)

# 1. Project Setup
project(DesktopEditors VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CORE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../core")
set(DESKTOP_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../desktop-sdk")

include(${CORE_ROOT_DIR}/common.cmake)

if(NOT TARGET PdfFile)
    add_subdirectory(${CORE_ROOT_DIR}/PdfFile PdfFile)
endif()

if(NOT TARGET XpsFile)
    add_subdirectory(${CORE_ROOT_DIR}/XpsFile XpsFile)
endif()

if(NOT TARGET DjVuFile)
    add_subdirectory(${CORE_ROOT_DIR}/DjVuFile DjVuFile)
endif()

if(NOT TARGET hunspell)
    add_subdirectory(${CORE_ROOT_DIR}/Common/3dParty/hunspell/qt hunspell)
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

message( "Fetch and build OpenSSL" )
set(OPENSSL_3RDPARTY_DIR "${CORE_ROOT_DIR}/Common/3dParty/openssl")
set(OPENSSL_INSTALL_DIR "${EO_CORE_3RD_PARTY_INSTALL_DIR}/openssl_build_linux_64")
get_filename_component(OPENSSL_3RDPARTY_DIR_ABS "${OPENSSL_3RDPARTY_DIR}" ABSOLUTE)
get_filename_component(OPENSSL_INSTALL_DIR_ABS "${OPENSSL_INSTALL_DIR}" ABSOLUTE)
execute_process(
    COMMAND_ECHO STDOUT
    COMMAND ./nc-build.sh "${EO_CORE_3RD_PARTY_WORK_DIR}/openssl" "${OPENSSL_INSTALL_DIR}"
    WORKING_DIRECTORY ${OPENSSL_3RDPARTY_DIR_ABS}
)


# 2. Find Qt Packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets PrintSupport Svg LinguistTools Multimedia MultimediaWidgets)

if(LINUX)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus X11Extras)
    add_definitions(-DLINUX -D_LINUX -D_X11)
    # Equivalent to x11extras for Qt6 if needed, or use X11 directly
endif()

# 3. Definitions & Global Config
add_definitions(-D__DONT_WRITE_IN_APP_TITLE)
add_definitions(-DAPP_ICON_PATH="./res/icons/desktopeditors.ico")
add_definitions(-DCOPYRIGHT_YEAR=2026) # Or use a variable

# Handle Product Version from Environment
set(ENV_PRODUCT_VERSION $ENV{PRODUCT_VERSION})
if(ENV_PRODUCT_VERSION)
    set(FULL_PRODUCT_VERSION "${ENV_PRODUCT_VERSION}.$ENV{BUILD_NUMBER}")
    add_definitions(-DVER_PRODUCT_VERSION=${FULL_PRODUCT_VERSION})
    string(REPLACE "." "," VERSION_COMMAS ${FULL_PRODUCT_VERSION})
    add_definitions(-DVER_PRODUCT_VERSION_COMMAS=${VERSION_COMMAS})
endif()

# 4. Source and Header Lists
set(COMMON_HEADERS
    src/prop/defines_p.h
    src/prop/cascapplicationmanagerwrapperintf.h
    src/prop/version_p.h
    src/prop/cmainwindowimpl.h
    src/windows/cmainwindow.h
    src/windows/cwindowbase.h
    src/windows/ceditorwindow.h
    src/windows/ceditorwindow_p.h
    src/windows/cpresenterwindow.h
    src/components/asctabwidget.h
    src/components/cdownloadwidget.h
    src/components/cpushbutton.h
    src/components/cfiledialog.h
    src/components/cprintprogress.h
    src/components/ctabbar.h
    src/components/cmessage.h
    src/components/canimatedicon.h
    src/components/ctabpanel.h
    src/components/csvgpushbutton.h
    src/components/celipsislabel.h
    src/components/cfullscrwidget.h
    src/components/cprintdialog.h
    src/components/ctooltip.h
    src/components/cmenu.h
    src/version.h
    src/defines.h
    src/ccefeventstransformer.h
    src/cascapplicationmanagerwrapper.h
    src/cascapplicationmanagerwrapper_private.h
    src/casctabdata.h
    src/utils.h
    src/chelp.h
    src/cfilechecker.h
    src/clogger.h
    src/clangater.h
    src/cprintdata.h
    src/cproviders.h
    src/cscalingwrapper.h
    src/ctabundockevent.h
    src/ccefeventsgate.h
    src/ceditortools.h
    src/cwindowsqueue.h
    src/ceventdriver.h
    src/cappeventfilter.h
    src/iconfactory.h
    src/cthemes.h
)

set(COMMON_SOURCES
    src/prop/cmainwindowimpl.cpp
    src/prop/utils.cpp
    src/windows/cmainwindow.cpp
    src/windows/cwindowbase.cpp
    src/windows/ceditorwindow.cpp
    src/windows/cpresenterwindow.cpp
    src/components/asctabwidget.cpp
    src/components/cdownloadwidget.cpp
    src/components/cpushbutton.cpp
    src/components/cfiledialog.cpp
    src/components/cprintprogress.cpp
    src/components/ctabbar.cpp
    src/components/cmessage.cpp
    src/components/canimatedicon.cpp
    src/components/ctabpanel.cpp
    src/components/csvgpushbutton.cpp
    src/components/celipsislabel.cpp
    src/components/cfullscrwidget.cpp
    src/components/cprintdialog.cpp
    src/components/ctooltip.cpp
    src/components/cmenu.cpp
    src/main.cpp
    src/ccefeventstransformer.cpp
    src/cascapplicationmanagerwrapper.cpp
    src/casctabdata.cpp
    src/chelp.cpp
    src/cfilechecker.cpp
    src/clogger.cpp
    src/clangater.cpp
    src/cprintdata.cpp
    src/cproviders.cpp
    src/cscalingwrapper.cpp
    src/ctabundockevent.cpp
    src/ccefeventsgate.cpp
    src/ceditortools.cpp
    src/ceventdriver.cpp
    src/cappeventfilter.cpp
    src/iconfactory.cpp
    src/cthemes.cpp
)


# 5. Conditional Updates Module
if(NOT BUILD_XP AND UPDMODULE)
    add_definitions(-D_UPDMODULE)
    list(APPEND COMMON_HEADERS src/cupdatemanager.h extras/update-daemon/src/classes/csocket.h)
    list(APPEND COMMON_SOURCES src/cupdatemanager.cpp extras/update-daemon/src/classes/csocket.cpp)
    include_directories(extras/update-daemon/src/classes)
endif()

# 6. Platform Specific Logic
if(WIN32)
    list(APPEND COMMON_SOURCES 
        src/windows/platform_win/cwindowplatform.cpp
        src/platform_win/singleapplication.cpp
        src/platform_win/association.cpp
        src/platform_win/filechooser.cpp
        src/platform_win/printdialog.cpp
    )
    list(APPEND COMMON_HEADERS 
        src/windows/platform_win/cwindowplatform.h
        src/windows/platform_win/caption.h
        src/platform_win/singleapplication.h
        src/platform_win/association.h
        src/platform_win/filechooser.h
        src/platform_win/printdialog.h
        src/platform_win/resource.h
    )
    set(PLATFORM_LIBS 
        wininet dnsapi version msimg32 ws2_32 usp10 psapi dbghelp 
        winmm shlwapi kernel32 gdi32 winspool comdlg32 advapi32 
        shell32 ole32 oleaut32 user32 uuid odbc32 odbccp32 delayimp 
        credui netapi32 comctl32 rpcrt4
    )
    
    if(BUILD_XP)
        add_definitions(-D__OS_WIN_XP)
    else()
        list(APPEND COMMON_SOURCES src/platform_win/wintoastlib.cpp src/components/cnotification.cpp)
    endif()

elseif(UNIX AND NOT APPLE)
    # Linux Resource Compiler (GResource)

    list(APPEND COMMON_SOURCES 
        src/windows/platform_linux/cx11decoration.cpp
        src/windows/platform_linux/cwindowplatform.cpp
        src/components/cnotification.cpp
        src/platform_linux/cdialogopenssl.cpp
        src/platform_linux/cdialogcertificateinfo.cpp
        src/platform_linux/singleapplication.cpp
        src/platform_linux/xdgdesktopportal.cpp
        src/platform_linux/gtkfilechooser.cpp
        src/platform_linux/gtkprintdialog.cpp
        src/platform_linux/gtkutils.cpp
        src/platform_linux/xcbutils.cpp
        extras/update-daemon/src/classes/csocket.cpp
    )
    list(APPEND COMMON_HEADERS 
        src/windows/platform_linux/cx11decoration.h
        #src/windows/platform_linux/gtk_addon.h
        src/windows/platform_linux/cwindowplatform.h
        src/components/cnotification.h
        src/platform_linux/cdialogopenssl.h
        src/platform_linux/cdialogcertificateinfo.h
        src/platform_linux/singleapplication.h
        src/platform_linux/xdgdesktopportal.h
        src/platform_linux/gtkfilechooser.h
        src/platform_linux/gtkprintdialog.h
        src/platform_linux/gtkutils.h
        src/platform_linux/xcbutils.h
        extras/update-daemon/src/classes/csocket.h
    )

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED glib-2.0 gtk+-3.0 atk gtk+-unix-print-3.0 xcb xext libnotify)

    # Custom command for glib-compile-resources
    set(GRESOURCE_XML ${CMAKE_CURRENT_SOURCE_DIR}/res/gresource.xml)
    set(GRESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/res/gresource.c)
    add_custom_command(
        OUTPUT ${GRESOURCE_C}
        COMMAND glib-compile-resources --target=${GRESOURCE_C} --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/res --generate-source ${GRESOURCE_XML}
        DEPENDS ${GRESOURCE_XML}
    )
    list(APPEND COMMON_SOURCES ${GRESOURCE_C})
    
    set(PLATFORM_LIBS ${GTK3_LIBRARIES} X11 X11-xcb cups)
endif()

# 7. Create Executable
qt_add_resources(RESOURCES resources.qrc)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extras.qrc")
    qt_add_resources(RESOURCES extras.qrc)
endif()

add_executable(DesktopEditors ${COMMON_SOURCES} ${COMMON_HEADERS} ${RESOURCES})

find_package(Boost REQUIRED COMPONENTS filesystem system)


find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)

# 8. Include Paths
target_include_directories(DesktopEditors PRIVATE
    src/prop
    src
    ${GTK3_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../desktop-sdk/ChromiumBasedEditors/lib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../desktop-sdk/ChromiumBasedEditors/lib/qt_wrapper/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/DesktopEditor
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/extras/update-daemon/src/classes
    ${OPENSSL_INSTALL_DIR}/include
)

# 9. Linking
target_link_libraries(DesktopEditors PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Multimedia 
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    ${PLATFORM_LIBS}
    ${DBUS_LIBRARIES}
    PdfFile
    DjVuFile
    XpsFile
    UnicodeConverter
    hunspell
    ooxmlsignature
    kernel
    kernel_network
    graphics
    ascdocumentscore
    qtascdocumentscore
    ${OPENSSL_INSTALL_DIR_ABS}/lib/libssl.a
    ${OPENSSL_INSTALL_DIR_ABS}/lib/libcrypto.a
)

if(LINUX)
    target_link_libraries(DesktopEditors PRIVATE
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::X11Extras
    )
endif()

# 10. Translations
file(GLOB TS_FILES "langs/*.ts")
qt5_create_translation(DesktopEditors TS_FILES ${TS_FILES})

#qt6: qt_add_translations...
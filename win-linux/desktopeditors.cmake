# SPDX-FileCopyrightText: 2026 Euro-Office contributors
# SPDX-License-Identifier: AGPL-3.0-only

# The application's sources and the platform libraries they need, shared by
# CMakeLists.txt and the unit test build in tests/fake-sdk, so both compile
# the same files. Sets COMMON_HEADERS, COMMON_SOURCES (absolute paths) and
# PLATFORM_LIBS.

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
    include_directories(${CMAKE_CURRENT_LIST_DIR}/extras/update-daemon/src/classes)
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
        src/windows/platform_linux/cplatformdecoration.cpp
        src/windows/platform_linux/cwindowplatform.cpp
        src/components/cnotification.cpp
        src/platform_linux/cdialogopenssl.cpp
        src/platform_linux/cdialogcertificateinfo.cpp
        src/platform_linux/singleapplication.cpp
        src/platform_linux/xdgdesktopportal.cpp
        src/platform_linux/gtkfilechooser.cpp
        src/platform_linux/gtkprintdialog.cpp
        src/platform_linux/gtkutils.cpp
        src/platform_linux/linux_window_utils.cpp
        src/platform_linux/x11backend.cpp
        src/platform_linux/waylandbackend.cpp
        src/platform_linux/platformbackendfactory.cpp
        extras/update-daemon/src/classes/csocket.cpp
    )
    list(APPEND COMMON_HEADERS 
        src/windows/platform_linux/cplatformdecoration.h
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
        src/platform_linux/linux_window_utils.h
        src/platform_linux/iplatformbackend.h
        src/platform_linux/x11backend.h
        src/platform_linux/waylandbackend.h
        extras/update-daemon/src/classes/csocket.h
    )

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED glib-2.0 gtk+-3.0 atk gtk+-unix-print-3.0 libnotify)
    pkg_check_modules(XCB_LIBS xcb xext)
    pkg_check_modules(X11_LIBS x11 x11-xcb)
    if(XCB_LIBS_FOUND AND X11_LIBS_FOUND)
        add_definitions(-DHAVE_X11)
    endif()
    add_definitions(-DHAVE_WAYLAND)

    # Custom command for glib-compile-resources
    set(GRESOURCE_XML ${CMAKE_CURRENT_LIST_DIR}/res/gresource.xml)
    set(GRESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/res/gresource.c)
    add_custom_command(
        OUTPUT ${GRESOURCE_C}
        COMMAND glib-compile-resources --target=${GRESOURCE_C} --sourcedir=${CMAKE_CURRENT_LIST_DIR}/res --generate-source ${GRESOURCE_XML}
        DEPENDS ${GRESOURCE_XML}
    )
    list(APPEND COMMON_SOURCES ${GRESOURCE_C})
    
    set(PLATFORM_LIBS ${GTK3_LIBRARIES} cups)
    if(XCB_LIBS_FOUND AND X11_LIBS_FOUND)
        list(APPEND PLATFORM_LIBS ${X11_LIBS_LIBRARIES} ${XCB_LIBS_LIBRARIES})
    endif()
endif()

# Absolute, so the lists also work from tests/fake-sdk
list(TRANSFORM COMMON_HEADERS PREPEND "${CMAKE_CURRENT_LIST_DIR}/" REGEX "^(src|extras)/")
list(TRANSFORM COMMON_SOURCES PREPEND "${CMAKE_CURRENT_LIST_DIR}/" REGEX "^(src|extras)/")

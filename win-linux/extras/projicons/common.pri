
QT      += core widgets
QT      -= gui

TARGET  = projicons
CONFIG  -= console
CONFIG  -= app_bundle
CONFIG  -= debug_and_release debug_and_release_target

TEMPLATE = app

INCLUDEPATH += $$PWD/src
HEADERS += $$PWD/src/version.h \
           $$PWD/src/resource.h
SOURCES += $$PWD/src/main.cpp
OTHER_FILES += $$PWD/version.rc \
               $$PWD/res/langs/translation.rc \
               $$PWD/res/manifest/projicons.exe.manifest

CONFIG += core_no_dst
include($$PWD/../../../../core/Common/base.pri)

ENV_PRODUCT_VERSION = $$(PRODUCT_VERSION)
!isEmpty(ENV_PRODUCT_VERSION) {
    FULL_PRODUCT_VERSION = $${ENV_PRODUCT_VERSION}.$$(BUILD_NUMBER)
    DEFINES += VER_PRODUCT_VERSION=$$FULL_PRODUCT_VERSION \
               VER_PRODUCT_VERSION_COMMAS=$$replace(FULL_PRODUCT_VERSION, \., ",")
}

CONFIG -= embed_manifest_exe
RC_FILE = $$PWD/version.rc

contains(QMAKE_TARGET.arch, x86_64):{
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
} else {
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
}

core_release:DESTDIR = $$DESTDIR/build
core_debug:DESTDIR = $$DESTDIR/build/debug

!isEmpty(OO_BUILD_BRANDING) {
    DESTDIR = $$DESTDIR/$$OO_BUILD_BRANDING
}

DESTDIR = $$DESTDIR/$$CORE_BUILDS_PLATFORM_PREFIX
build_xp {
    DESTDIR = $$DESTDIR/xp
    DEFINES += __OS_WIN_XP
} else {
    HEADERS += \
        $$PWD/src/jumplist.h \
        $$PWD/src/shellassoc.h

    SOURCES += \
        $$PWD/src/jumplist.cpp \
        $$PWD/src/shellassoc.cpp

    LIBS += -lshlwapi \
            -lshell32 \
            -ladvapi32 \
            -lcrypt32 \
            -lRpcrt4 \
            -lole32
}

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/rcc

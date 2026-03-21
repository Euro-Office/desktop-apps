
DESTDIR = $$PWD
include(common.pri)

DEFINES += COPYRIGHT_YEAR=$${CURRENT_YEAR}
DEFINES += APP_ICON_PATH=\"./icons/desktopeditors.ico\"
DEFINES += APP_LANG_PATH=\"./langs/langs.bin\"

ENV_URL_APPCAST_MAIN = $$(DESKTOP_URL_UPDATES_MAIN_CHANNEL)
!isEmpty(ENV_URL_APPCAST_MAIN) {
    DEFINES += URL_APPCAST_UPDATES=\\\"$${ENV_URL_APPCAST_MAIN}\\\"
}

ENV_URL_APPCAST_DEV = $$(DESKTOP_URL_UPDATES_DEV_CHANNEL)
!isEmpty(ENV_URL_APPCAST_DEV) {
    DEFINES += URL_APPCAST_DEV_CHANNEL=\\\"$${ENV_URL_APPCAST_DEV}\\\"
}

message(appcast url: $$ENV_URL_APPCAST_MAIN)
message(appcast dev url: $$ENV_URL_APPCAST_DEV)

core_linux {
    GLIB_RESOURCE_FILES += $$PWD/res/gresource.xml

    glib_resources.name = gresource
    glib_resources.input = GLIB_RESOURCE_FILES
    glib_resources.output = $$PWD/res/${QMAKE_FILE_IN_BASE}.c
    glib_resources.commands = glib-compile-resources --target ${QMAKE_FILE_OUT} --sourcedir ${QMAKE_FILE_IN_PATH} --generate-source ${QMAKE_FILE_IN}
    glib_resources.variable_out = SOURCES
    QMAKE_EXTRA_COMPILERS += glib_resources
}

OTHER_FILES += $$PWD/res/langs/langs.isl

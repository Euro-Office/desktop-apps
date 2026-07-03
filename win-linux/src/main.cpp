/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#ifdef _WIN32
# include "platform_win/singleapplication.h"
#else
# include <gtk/gtk.h>
# include "platform_linux/singleapplication.h"
# include "components/cmessage.h"
# include <unistd.h>
#endif
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "clangater.h"
#include "clogger.h"
#include "version.h"
#include "utils.h"
#include "chelp.h"
#include "common/File.h"
#include <QStyleFactory>
#include <vector>
#include <QGuiApplication>
#include <QDebug>


int main( int argc, char *argv[] )
{
    bool isWayland = false;
    int new_argc = argc;
    char** new_argv = argv;
    std::vector<char*> dynamic_argv;
#ifdef _WIN32
    Core_SetProcessDpiAwareness();
    Utils::setAppUserModelId();
    WCHAR * cm_line = GetCommandLine();
    InputArgs::init(cm_line);
    if ( InputArgs::contains(L"--assoc") ) {
        return 0;
    }
#else
    dynamic_argv.assign(argv, argv + argc);
    QByteArray platform = qgetenv("QT_QPA_PLATFORM");
    if (platform.isEmpty()) {
        QByteArray sessionType = qgetenv("XDG_SESSION_TYPE");
        if (sessionType == "wayland") {
            platform = "wayland";
        } else {
            platform = "xcb";
        }
        qputenv("QT_QPA_PLATFORM", platform);
    }
    isWayland = (platform == "wayland");

    if (isWayland) {
        qputenv("GDK_BACKEND", "wayland");
        dynamic_argv.push_back(const_cast<char*>("--ozone-platform=wayland"));
    } else {
        qputenv("GDK_BACKEND", "x11");
    }
    dynamic_argv.push_back(nullptr);
    new_argc = dynamic_argv.size() - 1;
    new_argv = dynamic_argv.data();

    InputArgs::init(new_argc, new_argv);
    if (geteuid() == 0) {
        CMessage::warning(nullptr, WARNING_LAUNCH_WITH_ADMIN_RIGHTS);
        return 0;
    }
    if ( InputArgs::contains(L"--set-instapp-port") ) {
        Utils::setInstAppPort(std::stoi(InputArgs::argument_value(L"--set-instapp-port")));
        return 0;
    }
#endif
#ifdef __linux
    char* qpaPlatform = getenv("QT_QPA_PLATFORM");
    char* xdgSessionType = getenv("XDG_SESSION_TYPE");
    if ((qpaPlatform && strcmp(qpaPlatform, "wayland") == 0) ||
        (xdgSessionType && strcmp(xdgSessionType, "wayland") == 0)) {
        isWayland = true;
    }
#endif

    if (!isWayland) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
#else
        QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#endif
    } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    }
    // On Wayland, skip AA_Use96Dpi to let the compositor's native DPI
    // take effect — otherwise Qt overrides DPI, causing fuzzy text.
    if (!isWayland)
        QCoreApplication::setAttribute(Qt::AA_Use96Dpi);
#ifdef _WIN32
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);   // avoid Qt's ANGLE colliding with CEF's libEGL/libGLESv2
#endif

    QCoreApplication::setApplicationName(QString::fromUtf8(WINDOW_NAME));
    QApplication::setApplicationDisplayName(QString::fromUtf8(WINDOW_NAME));

    QString user_data_path = Utils::getUserPath() + APP_DATA_PATH;
    auto setup_paths = [&user_data_path](CAscApplicationManager * manager) {
#ifdef _WIN32
        QString common_data_path = Utils::getAppCommonPath();
        if ( !common_data_path.isEmpty() ) {
            manager->m_oSettings.SetUserDataPath(common_data_path.toStdWString());
            manager->m_oSettings.user_templates_path = (common_data_path + "/templates").toStdWString();

            Utils::makepath(user_data_path.append("/data"));
            manager->m_oSettings.cookie_path = (user_data_path + "/cookie").toStdWString();
            manager->m_oSettings.recover_path = (user_data_path + "/recover").toStdWString();
            manager->m_oSettings.fonts_cache_info_path = (user_data_path + "/fonts").toStdWString();

            Utils::makepath(QString().fromStdWString(manager->m_oSettings.fonts_cache_info_path));
        } else
#else
#endif
        {
            manager->m_oSettings.SetUserDataPath(user_data_path.toStdWString());
        }
        std::wstring app_path = NSFile::GetProcessDirectory();
        manager->m_oSettings.spell_dictionaries_path    = app_path + L"/dictionaries";
        manager->m_oSettings.file_converter_path        = app_path + L"/converter";
        manager->m_oSettings.recover_path               = (user_data_path + "/recover").toStdWString();
        manager->m_oSettings.user_plugins_path          = (user_data_path + "/sdkjs-plugins").toStdWString();
        manager->m_oSettings.local_editors_path         = app_path + L"/editors/web-apps/apps/api/documents/index.html";
        manager->m_oSettings.system_templates_path      = app_path  + L"/converter/templates";
        manager->m_oSettings.additional_fonts_folder.push_back(app_path + L"/fonts");
        manager->m_oSettings.country = Utils::systemLocationCode().toStdString();
        manager->m_oSettings.connection_error_path      = app_path + L"/editors/webext/noconnect.html";
    };

    if ( InputArgs::contains(L"--version") ) {
        qWarning() << VER_PRODUCTNAME_STR << "ver." << VER_FILEVERSION_STR;
        return 0;
    } else
    if ( InputArgs::contains(L"--help") ) {
        CHelp::out();
        return 0;
    }
    if ( InputArgs::contains(L"--updates-reset") ) {
        GET_REGISTRY_USER(reg_user)
        reg_user.beginGroup("Updates");
        reg_user.remove("");
        reg_user.endGroup();
        reg_user.remove("autoUpdateMode");
    }
    if ( InputArgs::contains(L"--geometry=default") ) {
        GET_REGISTRY_USER(reg_user)
        reg_user.remove("maximized");
        reg_user.remove("position");
    }
    if ( InputArgs::contains(L"--lock-portals") ) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue("lockPortals", true);
    } else
    if ( InputArgs::contains(L"--unlock-portals") ) {
        GET_REGISTRY_USER(reg_user)
        reg_user.remove("lockPortals");
    }

    SingleApplication app(new_argc, new_argv);

    if ( !app.isPrimary() ) {
        QString _out_args;
        auto _args = InputArgs::arguments();
        if (_args.size() > 0) {
            foreach (auto w_arg, _args) {
                const QString arg = QString::fromStdWString(w_arg);
                if ( arg.startsWith("--new:") )
                    _out_args.append(arg).append(";");
                else
                if ( arg.mid(0,2) != "--" )
                    _out_args.append(arg + ";");
            }
        }
        bool res = app.sendMessage(_out_args.toUtf8());
        CLogger::log("The instance is not primary and will be closed. Parameter sending status: " + QString::number(res));
        return 0;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    app.setStyle(QStyleFactory::create("Fusion"));

    /* the order is important */
#ifdef __linux
    gtk_disable_setlocale();
    gtk_init(&new_argc, &new_argv);
#endif
    CApplicationCEF::Prepare(new_argc, new_argv);
    if (QGuiApplication::platformName() == "wayland") {
        qputenv("GDK_BACKEND", "wayland");
    }
    CApplicationCEF* application_cef = new CApplicationCEF();
    setup_paths(&AscAppManager::getInstance());
    application_cef->Init_CEF(&AscAppManager::getInstance(), new_argc, new_argv);
    /* ********************** */

//    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    reg_user.setFallbacksEnabled(false);

    /* read lang fom different places
     * cmd argument --lang:en apply the language one time
     * cmd argument --keeplang:en also keep the language for next sessions
    */
    CLangater::init();
    AscAppManager::initializeApp();
    AscAppManager::startApp();
    AscAppManager::getInstance().StartSpellChecker();
    AscAppManager::getInstance().StartKeyboardChecker();
    AscAppManager::getInstance().CheckFonts();

    bool bIsOwnMessageLoop = false;
    int exit_code = application_cef->RunMessageLoop(bIsOwnMessageLoop);
    if (!bIsOwnMessageLoop)
        exit_code = app.exec();

    AscAppManager::getInstance().CloseApplication();
    delete application_cef;
    return exit_code;
}

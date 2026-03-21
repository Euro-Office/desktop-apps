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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "cmainwindowimpl.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include "version.h"
#include "clangater.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QFile>

#define DEFAULT_LICENSE_NAME    "GNU AGPL v3"
#define DEFAULT_LICENSE_URL     URL_AGPL
#define LICENSE_FILE_NAME       "/EULA.txt"

CMainWindowImpl::CMainWindowImpl(const QRect &rect) :
    CMainWindow(rect)
{
    QObject::connect(CLangater::getInstance(), &CLangater::onLangChanged, std::bind(&CMainWindowImpl::refreshAboutVersion, this));
}

void CMainWindowImpl::refreshAboutVersion()
{
    QJsonObject _json_obj;

    auto _read_license_name = [](QString& path) -> QString {
        QFileInfo fi(path);
        QDir dir = fi.dir();
        QStringList files = dir.entryList(QStringList() << fi.fileName(),
                                          QDir::Files, QDir::Name | QDir::IgnoreCase);
        if (files.isEmpty())
            return QString();

        path = dir.filePath(files.first());
        QFile f(path);
        QString n;
        if ( f.exists() ) {
            if ( f.open(QIODevice::ReadOnly | QIODevice::Text )) {
                QTextStream stream(&f);
                n = stream.readLine().trimmed();
                f.close();
            }
        }

        return n;
    };

    QString _lic_path = QCoreApplication::applicationDirPath() + LICENSE_FILE_NAME;
    QString _lic_name = _read_license_name(_lic_path),
            _lic_url;
    if ( _lic_name.isEmpty() ) {
        _lic_path = QCoreApplication::applicationDirPath() + "/License.txt";
        if ( (_lic_name = _read_license_name(_lic_path)).isEmpty() ) {
            _lic_name = DEFAULT_LICENSE_NAME;
            _lic_url = DEFAULT_LICENSE_URL;
        }
    } else {
        _json_obj["commercial"] = _lic_name != DEFAULT_LICENSE_NAME;
    }

    if ( _lic_url.isEmpty() ) {
        _lic_url = QUrl::fromLocalFile(_lic_path).toString();
    }

    QString _license;
    if ( !(_lic_name.count() > 15) )
        _license = tr("Licensed under") + " &lt;a class=\"link\" onclick=\"window.open('" + _lic_url + "')\" draggable=\"false\" href=\"#\"&gt;" + _lic_name + "&lt;/a&gt;";
    else _license = "&lt;a class=\"link\" onclick=\"window.open('" + _lic_url + "')\" draggable=\"false\" href=\"#\"&gt;" + _lic_name + "&lt;/a&gt;";


    _json_obj["version"]    = VER_FILEVERSION_STR;
#ifdef Q_OS_WIN
# if defined(_M_ARM64)
    _json_obj["arch"] = "arm64";
# elif defined(_M_ARM)
    _json_obj["arch"] = "arm";
# elif defined(_M_X64)
    _json_obj["arch"] = "x64";
# elif defined(_M_IX86)
    _json_obj["arch"] = "x86";
# endif
#else
# if defined(__aarch64__)
    _json_obj["arch"] = "arm64";
# elif defined(__x86_64__)
    _json_obj["arch"] = "x64";
# endif
#endif
    _json_obj["edition"]    = _license;

#if defined(ABOUT_PAGE_APP_NAME)
    _json_obj["appname"]    = ABOUT_PAGE_APP_NAME;
#else
    // _json_obj["appname"]    = WINDOW_NAME;
    _json_obj["appname"]    = "ONLYOFFICE Desktop Editors";
#endif
    _json_obj["rights"]     = ABOUT_COPYRIGHT_STR;
    _json_obj["link"]       = URL_SITE;
//    _json_obj["changelog"]  = "https://github.com/ONLYOFFICE/DesktopEditors/blob/master/CHANGELOG.md";

    QString _package = QSettings(qApp->applicationDirPath() + "/converter/package.config", QSettings::IniFormat).value("package").toString();
    if ( !_package.isEmpty() )
        _json_obj["pkg"] = _package;

    AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "app:version", Utils::stringifyJson(_json_obj));

    _json_obj.empty();
    _json_obj.insert("locale",
        QJsonObject({
            {"current", CLangater::getCurrentLangCode()},
            {"langs", CLangater::availableLangsToJson()}
        })
    );

//    if ( !AscAppManager::IsUseSystemScaling() ) {
        _json_obj["uiscaling"] = Scaling::factorToScaling(AscAppManager::userSettings(L"force-scale"));
//    }

#ifndef __OS_WIN_XP
    _json_obj["uitheme"] = QString::fromStdWString(GetCurrentTheme().id());
#endif

    _json_obj["spellcheckdetect"] = AscAppManager::userSettings(L"spell-check-input-mode") != L"0" ? "auto" : "off";

    GET_REGISTRY_USER(reg_user);
    _json_obj["editorwindowmode"] = reg_user.value("editorWindowMode",false).toBool();
    _json_obj["usegpu"] = !(AscAppManager::userSettings(L"disable-gpu") == L"1");
    _json_obj["useai"] = !(AscAppManager::userSettings(L"disable-ai") == L"1");

#ifndef __OS_WIN_XP
    _json_obj["rtl"] = AscAppManager::isRtlEnabled();
#endif

    // Read update settings
#ifdef _UPDMODULE
    if ( Utils::updatesAllowed() ) {
        AscAppManager::sendCommandTo(0, "updates:turn", "on");
        _json_obj["updates"] = QJsonObject({{"mode", reg_user.value("autoUpdateMode","ask").toString()}});
    }
#endif

    AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "settings:init", Utils::stringifyJson(_json_obj));
    if ( InputArgs::contains(L"--ascdesktop-reveal-app-config") )
            AscAppManager::sendCommandTo( nullptr, "retrive:localoptions", "" );
}

void CMainWindowImpl::onLocalOptions(const QString& json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        QFile file(Utils::getAppCommonPath() + "/app.conf");
        file.open(QFile::WriteOnly);
        file.write(jdoc.toJson());
        file.close();
    }
}

void CMainWindowImpl::doOpenLocalFile(COpenOptions& opts)
{
    CMainWindow::doOpenLocalFile(opts);
}

QString CMainWindowImpl::getSaveMessage() const
{
    return CMainWindow::getSaveMessage();
}

void CMainWindowImpl::onLocalFileSaveAs(void * d)
{
    Q_UNUSED(d)
}

/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
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
        } else if (_lic_name == "GNU AFFERO GENERAL PUBLIC LICENSE") {
            QFile f(_lic_path);
            if ( f.exists() ) {
                if ( f.open(QIODevice::ReadOnly | QIODevice::Text )) {
                    QTextStream stream(&f);
                    QString n = stream.readLine();
                    n = stream.readLine().trimmed();
                    if (n.contains("Version 3")) {
                        _lic_name = DEFAULT_LICENSE_NAME;
                    }
                    f.close();
                }
            }
        }
    } else if (_lic_name == "ONLYOFFICE Desktop Enterprise") {
        _lic_name = tr("License Agreement");
        _json_obj["commercial"] = true;
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

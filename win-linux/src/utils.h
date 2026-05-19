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

#ifndef UTILS_H
#define UTILS_H

#include <QStringList>
#include <QFileInfo>
#include <QWidget>
#ifdef Q_OS_WIN
# include <Windows.h>
#endif
#include "components/cfullscrwidget.h"

#define PROCESSEVENTS() AscAppManager::getInstance().processEvents()
#define IsPackage(type) (AppOptions::packageType() == AppOptions::AppPackageType::type)

namespace InputArgs {
    auto init(int argc, char** const argv) -> void;
    auto init(wchar_t const * argv) -> void;
    auto contains(const std::wstring&) -> bool;
    auto argument_value(const std::wstring& param) -> std::wstring;
    auto arguments() -> const std::vector<std::wstring>&;

    auto webapps_params() -> const std::wstring&;
    auto set_webapps_params(const std::wstring&) -> void;
    auto change_webapps_param(const std::wstring& from, const std::wstring& to) -> const std::wstring&;
}

namespace EditorJSVariables {
    auto init() -> void;
    auto setVariable(const QString& name, const QString& var) -> void;
    auto setVariable(const QString& name, const QJsonObject& obj) -> void;
    auto setVariable(const QString& name, const QJsonArray& array) -> void;
    auto applyVariable(const QString& name, const QJsonObject& obj) -> void;
    auto toWString() -> std::wstring;
    auto apply() -> void;
}

namespace AppOptions {
    enum class AppPackageType {
        Unknown,
        ISS,
        MSI,
        Snap,
        Flatpack,
        Portable
    };

    auto packageType() -> AppPackageType;
}

namespace Scaling {
    auto scalingToFactor(const QString&) -> std::wstring;
    auto factorToScaling(const std::wstring&) -> QString;
}

class Utils {
public:
    static QStringList * getInputFiles(const QStringList& inlist);
    static QString lastPath(int type);
    static void keepLastPath(int type, const QString&);
    static QString getUserPath();
    static std::wstring systemUserName();
    static std::wstring appUserName();
    static QString getAppCommonPath();
    static QRect getScreenGeometry(const QPoint&);
    static void openUrl(const QString&);
    static void openFileLocation(const QString&);
    static QString getPortalName(const QString&);
    static double getScreenDpiRatio(int);
    static double getScreenDpiRatio(const QPoint&);
    static double getScreenDpiRatioByHWND(int);
    static double getScreenDpiRatioByWidget(QWidget*);
    static QScreen * screenAt(const QPoint&);
    static QString replaceBackslash(const QString&);
    static std::wstring normalizeAppProtocolUrl(const std::wstring &url);
    static void replaceAll(std::wstring& subject, const std::wstring& search, const std::wstring& replace);
    static bool isFileLocal(const QString&);
    static QString uniqFileName(const QString& path);
    static bool makepath(const QString&);
    static bool writeFile(const QString &filePath, const QByteArray &data);

    static QString systemLocationCode();
    static QIcon appIcon();

    static QString stringifyJson(const QJsonObject&);

    static QByteArray readStylesheets(std::vector<std::string> const &);
    static QByteArray readStylesheets(const QString&);
    static QJsonObject parseJsonString(const std::wstring&);
    static QJsonObject parseJsonFile(const QString&);
    static bool updatesAllowed();
    static void addToRecent(const std::wstring&);
    static void processMoreEvents(uint timeout = 60);

#ifdef _WIN32
    enum class WinVer : uchar {
        Undef, WinXP, WinVista, Win7, Win8, Win8_1, Win10, Win11
    };
    static WinVer getWinVersion();
    static QString GetCurrentUserSID();
    static bool isSessionInProgress();
    static void setSessionInProgress(bool);
    static void setAppUserModelId();
#else    
    static void setInstAppPort(int);
    static int getInstAppPort();
#endif
};

namespace WindowHelper {
#ifdef Q_OS_LINUX
    class CParentDisable
    {
        QWidget* m_parent = nullptr;
    public:
        CParentDisable(QWidget* parent);
        ~CParentDisable();

        void enable(bool enabled);
    };

//    auto check_button_state(Qt::MouseButton b) -> bool;
    enum DesktopEnv {
        UNITY = 0,
        GNOME,
        KDE,
        XFCE,
        CINNAMON,
        OTHER
    };
    auto getEnvInfo() -> int;
    auto useGtkDialog() -> bool;
#else
//    auto isWindowSystemDocked(HWND handle) -> bool;
//    auto correctWindowMinimumSize(HWND handle) -> void;
//    auto correctModalOrder(HWND windowhandle, HWND modalhandle) -> void;
//    auto adjustWindowRect(HWND, double, LPRECT) -> void;
    auto bringToTop(HWND) -> void;
    auto toggleLayoutDirection(HWND hwnd) -> void;
#endif

//    auto correctWindowMinimumSize(const QRect&, const QSize&) -> QSize;
    auto isLeftButtonPressed() -> bool;
    auto constructFullscreenWidget(QWidget * panel) -> CFullScrWidget *;
    auto useNativeDialog() -> bool;
    auto activeWindow() -> QWidget*;
    auto currentTopWindow() -> QWidget*;
    auto defaultWindowMaximizeState() -> bool;
}

#endif // UTILS_H

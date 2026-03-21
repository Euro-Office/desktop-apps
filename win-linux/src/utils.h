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

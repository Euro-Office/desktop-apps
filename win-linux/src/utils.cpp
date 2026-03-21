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

#ifdef __linux__
# include "platform_linux/gtkutils.h"
#endif
#include "utils.h"
#include "defines.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QRegularExpression>
#include <QApplication>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QStorageInfo>
#include <QPrinterInfo>
#include <QProcess>
#include "cascapplicationmanagerwrapper.h"
#include "qdpichecker.h"
#include "common/File.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
# include <QDesktopWidget>
#endif

#ifdef _WIN32
# include <QDesktopServices>
#include <windowsx.h>
# include <sddl.h>
# include <Lm.h>
#include "shlobj.h"
#include "lmcons.h"
#else
# include "platform_linux/xcbutils.h"
# include <QEventLoop>
# include <QX11Info>
#include <sys/stat.h>
#include <stdlib.h>
#endif


namespace InputArgs {
    std::vector<std::wstring> in_args;

    auto init(int argc, char** const argv) -> void {
        for (int c(1); c < argc; ++c) {
            in_args.push_back(UTF8_TO_U(std::string(argv[c])));
        }
    }

    auto init(wchar_t const * wargvl) -> void {
#ifdef Q_OS_WIN
        int argc;
        LPWSTR * argv = CommandLineToArgvW(wargvl, &argc);

        if (argv != nullptr) {
            for(int i(1); i < argc; ++i) {
                in_args.push_back(argv[i]);
            }
        }

        LocalFree(argv);
#endif
    }


    auto contains(const std::wstring& param) -> bool {
        const std::wstring::size_type l = param.length();
        auto iter = std::find_if(std::begin(in_args), std::end(in_args),
            [&param, l](const std::wstring& s) {
                std::wstring::size_type n = s.find(param);
                if (n != std::wstring::npos) {
                    if (n + l == s.length())
                        return true;
                    else {
                        wchar_t d = s.at(n+l);
                        if (d == L':' || d == L'=' || d == L'|')
                            return true;
                    }
                }

                return false;
        });

        return iter != end(in_args);
    }

    auto argument_value(const std::wstring& param) -> std::wstring {
        for (const auto& item: in_args) {
            if ( item.find(param) != std::wstring::npos ) {
                return item.substr(param.size() + 1); // substring after '=' or ':' symbol
            }
        }

        return L"";
    }

    auto arguments() -> const std::vector<std::wstring>& {
        return in_args;
    }

    std::wstring web_apps_params;
    auto webapps_params() -> const std::wstring& {
        return web_apps_params;
    }

    auto set_webapps_params(const std::wstring& params) -> void {
        web_apps_params = params;
    }

    auto change_webapps_param(const std::wstring& from, const std::wstring& to) -> const std::wstring& {
        size_t start_pos = web_apps_params.find(from);
        if( start_pos != std::string::npos ) {
            web_apps_params.replace(start_pos, from.length(), to);
        } else
        if ( true /*append*/ ) {
            web_apps_params.append(to);
        }

        return web_apps_params;
    }
}

namespace EditorJSVariables {
    QJsonObject vars_object;

    auto init() -> void {
#if defined(__OS_WIN_XP)
        vars_object["os"] = "winxp";
#elif defined(__linux__)
        vars_object["os"] = "linux";
#endif
        if ( InputArgs::contains(L"--help-url") )
            vars_object["helpUrl"] = QUrl(QString::fromStdWString(InputArgs::argument_value(L"--help-url"))).toString();
        else {
            GET_REGISTRY_USER(_reg_user)

            if ( _reg_user.contains("helpUrl") )
                vars_object["helpUrl"] = _reg_user.value("helpUrl").toString();
#ifdef URL_WEBAPPS_HELP
            else if ( !QString(URL_WEBAPPS_HELP).isEmpty() )
                vars_object["helpUrl"] = URL_WEBAPPS_HELP;
#endif
        }
        vars_object["defaultPrinterName"] = QPrinterInfo::defaultPrinterName();
    }

    auto setVariable(const QString& name, const QString& var) -> void {
        vars_object[name] = var;
    }

    auto setVariable(const QString& name, const QJsonObject& obj) -> void {
        vars_object[name] = obj;
    }

    auto setVariable(const QString& name, const QJsonArray& array) -> void {
        vars_object[name] = array;
    }

    auto applyVariable(const QString& name, const QJsonObject& obj) -> void {
        vars_object[name] = obj;
        apply();
    }

    auto toWString() -> std::wstring {
        return vars_object.isEmpty() ? L"" : Utils::stringifyJson(vars_object).toStdWString();
    }

    auto apply() -> void {
        AscAppManager::getInstance().SetRendererProcessVariable(toWString());
    }
}

namespace AppOptions {
    auto packageType() -> AppPackageType {
        QString _package = QSettings("./converter/package.config", QSettings::IniFormat).value("package").toString();

        if ( _package == "exe" ) return AppPackageType::ISS; else
        if ( _package == "msi" ) return AppPackageType::MSI; else
        if ( _package == "snap" ) return AppPackageType::Snap; else
        if ( _package == "flatpack" ) return AppPackageType::Flatpack; else
        /*if( _package.isEmpty() )*/ return AppPackageType::Portable;

        return AppPackageType::Unknown;
    }
}

namespace Scaling {
    auto scalingToFactor(const QString& scaling) -> std::wstring
    {
        switch ( scaling.toInt() ) {
        case 100: return L"1";
        case 125: return L"1.25";
        case 150: return L"1.5";
        case 175: return L"1.75";
        case 200: return L"2";
        case 225: return L"2.25";
        case 250: return L"2.5";
        case 275: return L"2.75";
        case 300: return L"3";
        case 350: return L"3.5";
        case 400: return L"4";
        case 450: return L"4.5";
        case 500: return L"5";
        default: return L"0";
        }
    }

    auto factorToScaling(const std::wstring& value) -> QString
    {
        return value == L"1" ? "100" :
               value == L"1.25" ? "125" :
               value == L"1.5" ? "150" :
               value == L"1.75" ? "175" :
               value == L"2" ? "200" :
               value == L"2.25" ? "225" :
               value == L"2.5" ? "250" :
               value == L"2.75" ? "275" :
               value == L"3" ? "300" :
               value == L"3.5" ? "350" :
               value == L"4" ? "400" :
               value == L"4.5" ? "450" :
               value == L"5" ? "500" : "0";
    }
}

QStringList * Utils::getInputFiles(const QStringList& inlist)
{
    QStringList * _ret_files_list = nullptr;

    if ( !inlist.isEmpty() ) {
        _ret_files_list = new QStringList;

        QStringListIterator i(inlist);
        while (i.hasNext()) {
            QString arg = i.next();

            if ( arg.startsWith("--new:") || arg.startsWith("--new=") )
                _ret_files_list->append( arg );
            else {
                QFileInfo info( arg );
                if ( info.isFile() ) {
                    _ret_files_list->append(info.absoluteFilePath());
                }
            }
        }
    }

    return _ret_files_list;
}

QString Utils::lastPath(int t)
{
    GET_REGISTRY_USER(_reg_user)

    QString _path;
    if (t == LOCAL_PATH_OPEN)
        _path = _reg_user.value("openPath").value<QString>(); else
        _path = _reg_user.value("savePath").value<QString>();

    return !_path.isEmpty() && QDir(_path).exists() ?
        _path : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void Utils::keepLastPath(int t, const QString& p)
{
    GET_REGISTRY_USER(_reg_user)
    if (t == LOCAL_PATH_OPEN)
        _reg_user.setValue("openPath", p); else
        _reg_user.setValue("savePath", p);
}

bool Utils::makepath(const QString& p)
{
#ifdef __linux
    mode_t _mask = umask(0);
    (_mask & S_IRWXO) && umask(_mask & ~S_IRWXO);
#endif
    return QDir().mkpath(p);
}

bool Utils::writeFile(const QString &filePath, const QByteArray &data)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        auto bytes_written = file.write(data);
        file.close();
        return bytes_written == data.size();
    }
    return false;
}

QRect Utils::getScreenGeometry(const QPoint& leftTop)
{
//    int _scr_num = QApplication::desktop()->screenNumber(leftTop); - return the wrong number
//    return QApplication::desktop()->screenGeometry(_scr_num);
#ifdef __linux
    auto pointToRect = [](const QPoint &p, const QRect &r) -> int {
        int dx = 0, dy = 0;
        if (p.x() < r.left()) dx = r.left() - p.x(); else
        if (p.x() > r.right()) dx = p.x() - r.right();

        if (p.y() < r.top()) dy = r.top() - p.y(); else
        if (p.y() > r.bottom()) dy = p.y() - r.bottom();

        return dx + dy;
    };

    int closestScreen = 0;
    int shortestDistance = pointToRect(leftTop, QApplication::desktop()->screenGeometry(0));

    for (int i = 0; ++i < QApplication::desktop()->screenCount(); ) {
        int thisDistance = pointToRect(leftTop, QApplication::desktop()->screenGeometry(i));
        if (thisDistance < shortestDistance) {
            shortestDistance = thisDistance;
            closestScreen = i;
        }
    }

    return QApplication::desktop()->screenGeometry(closestScreen);
#else
    POINT lt{leftTop.x(), leftTop.y()};
    MONITORINFO mi{sizeof(MONITORINFO)};
    ::GetMonitorInfo(::MonitorFromPoint(lt, MONITOR_DEFAULTTOPRIMARY), &mi);

    return QRect(QPoint(mi.rcWork.left, mi.rcWork.top), QPoint(mi.rcWork.right, mi.rcWork.bottom));
#endif
}

QString Utils::systemLocationCode()
{
#define LOCATION_MAX_LEN 9
#ifdef _WIN32
    WCHAR _country_code[LOCATION_MAX_LEN]{0};
    // "no entry point for GetLocaleInfoEx" error on win_xp
//    if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA ) {
//        if (!GetLocaleInfoEx(LOCALE_NAME_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, _country_code, LOCATION_MAX_LEN))
//            return "unknown";
//    } else {
        if (!GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, _country_code, LOCATION_MAX_LEN))
            return "unknown";
//    }

    return QString::fromWCharArray(_country_code);
#else
    QStringList list = QLocale().name().split('_');
    if (list.size() < 2)
        return "EN";

    return list.at(1);
#endif
}

void Utils::openUrl(const QString& url)
{
#ifdef __linux
    QUrl _url(url);
    if ( _url.scheme() == "mailto" ) {
        system(QString("LD_LIBRARY_PATH='' xdg-email %1")                   // xdg-email filepath email
                            .arg(QString( _url.toEncoded() )).toUtf8());
    } else {
		if (url.startsWith("xdg:")) {
			// url is already encoded for xdg
			std::wstring sUrlW = url.toStdWString().substr(4);
			std::string sCommand = "LD_LIBRARY_PATH='' xdg-open " + U_TO_UTF8(sUrlW);
			system(sCommand.c_str());
		} else {
			// xdg-open workingpath path
			system(QString("LD_LIBRARY_PATH='' xdg-open \"%1\"")
			       .arg(QString( _url.toEncoded() )).toUtf8());


		}
    }
#else
    QDesktopServices::openUrl(QUrl(url));
#endif
}

void Utils::openFileLocation(const QString& path)
{
#if defined(Q_OS_WIN)
    auto _path = QDir::toNativeSeparators(path).toStdWString();
    if (LPITEMIDLIST idl = ILCreateFromPath(_path.c_str())) {
        SHOpenFolderAndSelectItems(static_cast<LPCITEMIDLIST>(idl), 0, 0, 0);
        ILFree(idl);
    }
#else
    static QString _file_browser;
    static QString _arg_select = "--no-desktop";
    if ( _file_browser.isEmpty() ) {
        auto _get_cmd_output = [](const QString& cmd, const QStringList& args, QString& error) {
            QProcess process;
            process.start(cmd, args);
            process.waitForFinished(-1);

            error = process.readAllStandardError();
            return process.readAllStandardOutput();
        };

        QString _error;
        QString _cmd_output = _get_cmd_output("xdg-mime", QStringList{"query", "default", "inode/directory"}, _error);

        //    if ( _error.isEmpty() )
        {
            if ( _cmd_output.contains(QRegularExpression("nautilus\\.desktop", QRegularExpression::CaseInsensitiveOption)) ||
                    _cmd_output.contains(QRegularExpression("nautilus-folder-handler\\.desktop", QRegularExpression::CaseInsensitiveOption)) )
            {
//                _cmd_output = _get_cmd_output("nautilus", QStringList{"--version"}, _error);

        //        if ( _error.isEmpty() )
                {
//                    QRegularExpression _regex("nautilus\\s(\\d{1,3})\\.(\\d{1,3})(?:\\.(\\d{1,5}))?");
//                    QRegularExpressionMatch match = _regex.match(_cmd_output);
//                    if ( match.hasMatch() ) {
//                        bool is_verion_supported = match.captured(1).toInt() > 3;
//                        if ( !is_verion_supported && match.captured(1).toInt() == 3 ) {
//                            is_verion_supported = match.captured(2).toInt() > 0;

//                            if ( !is_verion_supported && !match.captured(3).isEmpty() )
//                                is_verion_supported = !(match.captured(3).toInt() < 2);
//                        }

//                        is_file_browser_supported = is_verion_supported;
//                    }
                }

                _file_browser = "nautilus";
            } else
            if ( _cmd_output.contains(QRegularExpression("dolphin\\.desktop", QRegularExpression::CaseInsensitiveOption)) ) {
                _file_browser = "dolphin";
                _arg_select = "--select";
            } else
            if ( _cmd_output == "caja-folder-handler.desktop" ) {
                _file_browser = "caja";
            } else
            if ( _cmd_output == "nemo.desktop" ) {
                _file_browser = "nemo";
            } else
            if ( _cmd_output == "kfmclient_dir.desktop" ) {
                _file_browser = "konqueror";
                _arg_select = "--select";
            } else {
                _file_browser = "unknown";
            }
        }
    }

    QFileInfo fileInfo(path);
    if ( !_file_browser.isEmpty() && _file_browser != "unknown" ) {
        qputenv("LD_LIBRARY_PATH", "");
        QProcess::startDetached(_file_browser, QStringList{_arg_select, fileInfo.absoluteFilePath()});
    } else
        system(QString("LD_LIBRARY_PATH='' xdg-open \"%1\"").arg(fileInfo.path()).toUtf8());
#endif
}

bool Utils::isFileLocal(const QString& path)
{
    QStorageInfo storage(QFileInfo(path).dir());

# ifdef Q_OS_WIN
    return storage.device().startsWith("\\\\?\\");
# else
    return storage.device().startsWith("/dev/");
# endif
}

QString Utils::uniqFileName(const QString& path)
{
    QFileInfo _info(path);

    if ( _info.exists() ) {
        QString _name = _info.baseName(),
                _suffix = _info.suffix();
        QDir _dir = _info.dir();

        int _index{0};
        while ( true ) {
            _info = QFileInfo(_dir, _name + QString::number(++_index) + "." + _suffix);

            if ( !_info.exists() ) return _info.absoluteFilePath();
        }
    }

    return path;
}

QString Utils::getPortalName(const QString& url)
{
    if ( !url.isEmpty() ) {
        QRegularExpressionMatch match = QRegularExpression(rePortalName).match(url);
        if (match.hasMatch()) {
            QString out = match.captured(1);
            return out.endsWith('/') ? out.remove(-1, 1) : out;
        }
    }

    return url;
}

QString Utils::stringifyJson(const QJsonObject& obj)
{
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

inline double choose_scaling(double s)
{
    return s > 4.5 ? 5 :
           s > 4 ? 4.5 :
           s > 3.5 ? 4 :
           s > 3 ? 3.5 :
           s > 2.75 ? 3 :
           s > 2.5 ? 2.75 :
           s > 2.25 ? 2.5 :
           s > 2 ? 2.25 :
           s > 1.75 ? 2 :
           s > 1.5 ? 1.75 :
           s > 1.25 ? 1.5 :
           s > 1 ? 1.25 : 1;
}

double Utils::getScreenDpiRatio(int scrnum)
{
    unsigned int _dpi_x = 0;
    unsigned int _dpi_y = 0;
    double nScale = AscAppManager::getInstance().GetMonitorScaleByIndex(scrnum, _dpi_x, _dpi_y);
    return choose_scaling(nScale);
}

double Utils::getScreenDpiRatio(const QPoint& pt)
{
    QWidget _w;
    _w.setGeometry(QRect(pt, QSize(10,10)));

#ifdef Q_OS_LINUX
    return getScreenDpiRatioByWidget(&_w);
#else
    return getScreenDpiRatioByHWND(_w.winId());
#endif
}

double Utils::getScreenDpiRatioByHWND(int hwnd)
{
    unsigned int _dpi_x = 0;
    unsigned int _dpi_y = 0;
    double nScale = AscAppManager::getInstance().GetMonitorScaleByWindow((WindowHandleId)hwnd, _dpi_x, _dpi_y);
    return choose_scaling(nScale);
}

double Utils::getScreenDpiRatioByWidget(QWidget* wid)
{
    if (!wid)
        return 1;

    unsigned int nDpiX = 0;
    unsigned int nDpiY = 0;

#ifdef Q_OS_LINUX
    QDpiChecker * pChecker = (QDpiChecker *)AscAppManager::getInstance().GetDpiChecker();
    int nResult = pChecker->GetWidgetDpi(wid, &nDpiX, &nDpiY);
    double dpiApp = pChecker->GetScale(nDpiX, nDpiY);
#else
    double dpiApp = AscAppManager::getInstance().GetMonitorScaleByWindow((WindowHandleId)wid->winId(), nDpiX, nDpiY);
#endif

    if ( dpiApp >= 0 ) {
        return choose_scaling(dpiApp);
    }

    return wid->devicePixelRatio();
}

QScreen * Utils::screenAt(const QPoint& pt)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    return QApplication::screenAt(pt);
#else
    QVarLengthArray<const QScreen *, 8> _cached_screens;
    for (const QScreen *screen : QApplication::screens()) {
        if (_cached_screens.contains(screen))
            continue;

        for (QScreen *sibling : screen->virtualSiblings()) {
            QRect r = sibling->geometry();
            if (sibling->geometry().contains(pt))
                return sibling;

            _cached_screens.append(sibling);
        }
    }

    return nullptr;
#endif
}

/*
QByteArray Utils::getAppStylesheets(int scale)
{
    auto read_styles = [](const QString& dir) {
        QByteArray _css;
        QFile file;
        QFileInfoList files = QDir(dir).entryInfoList(QStringList("*.qss"), QDir::Files);
        foreach(const QFileInfo &info, files) {
            file.setFileName(info.absoluteFilePath());
            if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
                _css.append(file.readAll());
                file.close();
            }
        }

        return _css;
    };

    QByteArray _out( read_styles(QString(":styles/res/styles")) );

    if ( scale > 1 ) {
        _out.append( read_styles(QString(":styles@2x")) );
    }

    return _out;
}
*/

QByteArray Utils::readStylesheets(std::vector<std::string> const &list)
{
    QByteArray _css;
    for ( auto &path : list ) {
        _css.append(readStylesheets(QString::fromStdString(path)));
    }

    return _css;
}

QByteArray Utils::readStylesheets(const QString& path)
{
    QByteArray _css;
    QFile file(path);
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        _css.append(file.readAll());
        file.close();
    }

    return _css;
}

QJsonObject Utils::parseJsonString(const std::wstring& wjson)
{
    QJsonParseError jerror;
    QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
    QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        return jdoc.object();
    }

    return QJsonObject();
}

QJsonObject Utils::parseJsonFile(const QString& path)
{
    QFile file(path);
    if ( file.open(QIODevice::ReadOnly) ) {
        QByteArray data{file.readAll()};
        file.close();

        QJsonParseError jpe;
        QJsonDocument jdoc = QJsonDocument::fromJson(data, &jpe);
        if ( jpe.error == QJsonParseError::NoError ) {
            return jdoc.object();
        }
    }

    return QJsonObject();
}

bool Utils::updatesAllowed()
{
    GET_REGISTRY_SYSTEM(reg_system)
    if (reg_system.value("CheckForUpdates", true).toBool()) {
#ifdef _WIN32
        return (IsPackage(Portable) || IsPackage(ISS) || IsPackage(MSI));
#else
        return IsPackage(Portable);
#endif
    }
    return false;
}

void Utils::addToRecent(const std::wstring &path)
{
#ifdef _WIN32
    std::wstring _path(path);
    std::replace(_path.begin(), _path.end(), '/', '\\');
# ifdef __OS_WIN_XP
    SHAddToRecentDocs(SHARD_PATH, _path.c_str());
# else
    if (LPITEMIDLIST idl = ILCreateFromPath(_path.c_str())) {
        SHARDAPPIDINFOIDLIST inf;
        inf.pidl = static_cast<PCIDLIST_ABSOLUTE>(idl);
        inf.pszAppID = TEXT(APP_USER_MODEL_ID);
        SHAddToRecentDocs(SHARD_APPIDINFOIDLIST, &inf);
        ILFree(idl);
    }
# endif
#else
    QString _path = QString::fromStdWString(path);
    std::string uri = "file://" + _path.toStdString();
    add_to_recent(uri.c_str());
#endif
}

void Utils::processMoreEvents(uint timeout)
{
    QEventLoop loop;
    QTimer::singleShot(timeout, &loop, SLOT(quit()));
    loop.exec();
}

#ifdef _WIN32
Utils::WinVer Utils::getWinVersion()
{
    static WinVer winVer = WinVer::Undef;
    if (winVer == WinVer::Undef) {
        if (HMODULE module = GetModuleHandleA("ntdll")) {
            NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
            *(FARPROC*)&RtlGetVersion = GetProcAddress(module, "RtlGetVersion");
            if (RtlGetVersion) {
                OSVERSIONINFOEXW os = {0};
                os.dwOSVersionInfoSize = sizeof(os);
                RtlGetVersion(&os);
#define MjrVer os.dwMajorVersion
#define MinVer os.dwMinorVersion
#define BldVer os.dwBuildNumber
                winVer = MjrVer == 5L && (MinVer == 1L || MinVer == 2L) ? WinVer::WinXP :
                         MjrVer == 6L && MinVer == 0L ? WinVer::WinVista :
                         MjrVer == 6L && MinVer == 1L ? WinVer::Win7 :
                         MjrVer == 6L && MinVer == 2L ? WinVer::Win8 :
                         MjrVer == 6L && MinVer == 3L ? WinVer::Win8_1 :
                         MjrVer == 10L && MinVer == 0L && BldVer < 22000 ? WinVer::Win10 :
                         MjrVer == 10L && MinVer == 0L && BldVer >= 22000 ? WinVer::Win11 :
                         MjrVer == 10L && MinVer > 0L ? WinVer::Win11 :
                         MjrVer > 10L ? WinVer::Win11 : WinVer::Undef;
            }
        }
    }
    return winVer;
}

QString Utils::GetCurrentUserSID()
{
    static QString user_sid;
    if (user_sid.isEmpty()) {
        HANDLE hToken = NULL;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            DWORD tokenLen = 0;
            GetTokenInformation(hToken, TokenUser, NULL, 0, &tokenLen);
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(tokenLen)) {
                    if (GetTokenInformation(hToken, TokenUser, pTokenUser, tokenLen, &tokenLen)) {
                        LPWSTR sid = NULL;
                        if (ConvertSidToStringSid(pTokenUser->User.Sid, &sid)) {
                            user_sid = QString::fromWCharArray(sid);
                            LocalFree(sid);
                        }
                    }
                    free(pTokenUser);
                }
            }
            CloseHandle(hToken);
        }
    }
    return user_sid;
}

std::atomic_bool sessionInProgress{true};

bool Utils::isSessionInProgress()
{
    return sessionInProgress;
}

void Utils::setSessionInProgress(bool state)
{
    sessionInProgress = state;
}

void Utils::setAppUserModelId()
{
    if (HMODULE lib = LoadLibrary(L"shell32")) {
        HRESULT (WINAPI *SetAppUserModelID)(PCWSTR AppID);
        *(FARPROC*)&SetAppUserModelID = GetProcAddress(lib, "SetCurrentProcessExplicitAppUserModelID");
        if (SetAppUserModelID)
            SetAppUserModelID(TEXT(APP_USER_MODEL_ID));
        FreeLibrary(lib);
    }
}
#else
void Utils::setInstAppPort(int port)
{
    GET_REGISTRY_USER(reg_user);
    if (port == -1) {
        reg_user.remove("instAppPort");
    } else
    if (port > 1023 && port < 65536) {
        reg_user.setValue("instAppPort", port);
    } else {
        qWarning() << "Value not applied: port must be in the range 1024 - 65535";
    }
}

int Utils::getInstAppPort()
{
    GET_REGISTRY_USER(reg_user);
    return reg_user.value("instAppPort", INSTANCE_APP_PORT).toInt();
}
#endif

QString Utils::replaceBackslash(const QString& path)
{
    return QString(path).replace(QRegularExpression("\\\\"), "/");
}

std::wstring Utils::normalizeAppProtocolUrl(const std::wstring &url)
{
    QUrl _url(QString::fromStdWString(url));
    if (_url.scheme() == APP_PROTOCOL) {
        QUrlQuery query(_url);
        query.addQueryItem("placement", "desktop");
        _url.setQuery(query);
        return _url.toString(QUrl::RemoveScheme).toStdWString();
    } else
    if (_url.scheme() == "ascdesktop") {
        if (_url.host() == "external") {
            QString path = _url.path();
            if (path.startsWith('/'))
                path = path.mid(1);
            return path.toStdWString();
        }
    }
    return url;
}

void Utils::replaceAll(std::wstring& subject, const std::wstring& search, const std::wstring& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::wstring::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

std::wstring Utils::systemUserName()
{
#ifdef Q_OS_WIN
    WCHAR _env_name[UNLEN + 1]{0};
    DWORD _size = UNLEN + 1;

    if (GetUserName(_env_name, &_size)) {
        LPBYTE buff = nullptr;
        if (NetUserGetInfo(nullptr, _env_name, 10, &buff) == NERR_Success) {
            std::wstring user_name(reinterpret_cast<USER_INFO_10*>(buff)->usri10_full_name);
            NetApiBufferFree(buff);
            if (!user_name.empty())
                return user_name;
        }
        return _env_name;
    }
    return L"Unknown.User";
#else
    QString _env_name = qgetenv("USER");
    if ( _env_name.isEmpty() ) {
        _env_name = qgetenv("USERNAME");

        if (_env_name.isEmpty())
            _env_name = "Unknown.User";
    }

    return _env_name.toStdWString();
#endif
}

std::wstring Utils::appUserName()
{
    GET_REGISTRY_USER(_reg_user)

    QJsonParseError jerror;
    QByteArray data = QByteArray::fromBase64(_reg_user.value("appdata").toByteArray());
    QJsonDocument jdoc = QJsonDocument::fromJson(data, &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        QJsonObject objRoot = jdoc.object();

        if ( objRoot.contains("username") ) {
            return objRoot["username"].toString().toStdWString();
        }
    }

    return systemUserName();
}


namespace WindowHelper {
#ifdef Q_OS_LINUX
    CParentDisable::CParentDisable(QWidget* parent) : m_parent(parent)
    {
        enable(false);
    }

    CParentDisable::~CParentDisable()
    {
        enable(true);
    }

    void CParentDisable::enable(bool enabled)
    {
        CWindowBase *wb = dynamic_cast<CWindowBase*>(m_parent);
        if (!wb) return;

        wb->setEnabled(enabled);
        WId wnd = wb->mainPanel()->winId();
        XcbUtils::setInputEnabled(wnd, enabled);
    }

    // Linux Environment Info
    int desktop_env = -1;

    auto getEnvInfo() -> int {
        if ( desktop_env == -1 ) {
            const QString env(qgetenv("XDG_CURRENT_DESKTOP"));
            if (env.indexOf("Unity") != -1) {
                const QString session(qgetenv("DESKTOP_SESSION"));
                desktop_env = (session.indexOf("gnome-fallback") != -1) ? GNOME : UNITY;
            } else
            if (env.indexOf("GNOME") != -1)
                desktop_env = GNOME;
            else
            if (env.indexOf("KDE") != -1)
                desktop_env = KDE;
            else
            if (env.indexOf("XFCE") != -1)
                desktop_env = XFCE;
            else
            if (env.indexOf("Cinnamon") != -1)
                desktop_env = CINNAMON;
            else desktop_env = OTHER;
        }
        return desktop_env;
    }

    auto useGtkDialog() -> bool {
        GET_REGISTRY_USER(reg_user)
        bool use_gtk_dialog = true;
        bool saved_flag = reg_user.value("--xdg-desktop-portal", false).toBool();
        if (InputArgs::contains(L"--xdg-desktop-portal=default")) {
            use_gtk_dialog = false;
            if (saved_flag)
                reg_user.setValue("--xdg-desktop-portal", false);
        } else
        if (InputArgs::contains(L"--xdg-desktop-portal")) {
            use_gtk_dialog = false;
            if (!saved_flag)
                reg_user.setValue("--xdg-desktop-portal", true);
        } else {
            if (saved_flag)
                use_gtk_dialog = false;
        }
        return use_gtk_dialog;
    }
#else
//    auto isWindowSystemDocked(HWND handle) -> bool {
//        RECT windowrect;
//        WINDOWPLACEMENT wp; wp.length = sizeof(WINDOWPLACEMENT);
//        if ( GetWindowRect(handle, &windowrect) && GetWindowPlacement(handle, &wp) && wp.showCmd == SW_SHOWNORMAL ) {
//            return (wp.rcNormalPosition.right - wp.rcNormalPosition.left != windowrect.right - windowrect.left) ||
//                        (wp.rcNormalPosition.bottom - wp.rcNormalPosition.top != windowrect.bottom - windowrect.top);
//        }

//        return false;
//    }

//     auto correctWindowMinimumSize(HWND handle) -> void {
//         WINDOWPLACEMENT wp; wp.length = sizeof(WINDOWPLACEMENT);
//         if ( GetWindowPlacement(handle, &wp) ) {
//             int dpi_ratio = Utils::getScreenDpiRatioByHWND((int)handle);
//             QSize _min_windowsize{MAIN_WINDOW_MIN_WIDTH * dpi_ratio,MAIN_WINDOW_MIN_HEIGHT * dpi_ratio};
//             QRect windowRect{QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top),
//                                     QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom)};

//             if ( windowRect.width() < _min_windowsize.width() ||
//                     windowRect.height() < _min_windowsize.height() )
//             {
// //                if ( windowRect.width() < _min_windowsize.width() )
//                     wp.rcNormalPosition.right = wp.rcNormalPosition.left + _min_windowsize.width();

// //                if ( windowRect.height() < _min_windowsize.height() )
//                     wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + _min_windowsize.height();

//                 SetWindowPlacement(handle, &wp);
//             }
//         }
//     }

//    auto correctModalOrder(HWND windowhandle, HWND modalhandle) -> void
//    {
//        if ( !IsWindowEnabled(windowhandle) && modalhandle && modalhandle != windowhandle ) {
//            SetActiveWindow(modalhandle);
//            SetWindowPos(windowhandle, modalhandle, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
//        }
//    }

//    typedef BOOL (__stdcall *AdjustWindowRectExForDpiW)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
//    auto adjustWindowRect(HWND handle, double dpiratio, LPRECT rect) -> void
//    {
//        static AdjustWindowRectExForDpiW _adjustWindowRectEx = nullptr;
//        static bool _is_read = false;
//        if ( !_is_read && !_adjustWindowRectEx ) {
//            HMODULE _lib = ::LoadLibrary(L"user32.dll");
//            _adjustWindowRectEx = reinterpret_cast<AdjustWindowRectExForDpiW>(GetProcAddress(_lib, "AdjustWindowRectExForDpi"));
//            FreeLibrary(_lib);

//            _is_read = true;
//        }

//        if ( _adjustWindowRectEx ) {
//            _adjustWindowRectEx(rect, (GetWindowStyle(handle) & ~WS_DLGFRAME), FALSE, 0, 96*dpiratio);
//        } else AdjustWindowRectEx(rect, (GetWindowStyle(handle) & ~WS_DLGFRAME), FALSE, 0);
//    }

    auto bringToTop(HWND hwnd) -> void
    {
        DWORD appID = ::GetCurrentThreadId();
        DWORD frgID = ::GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
        ::AttachThreadInput(frgID, appID, TRUE);
        ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
        ::SetForegroundWindow(hwnd);
        ::SetFocus(hwnd);
        ::SetActiveWindow(hwnd);
        ::AttachThreadInput(frgID, appID, FALSE);
    }

    auto toggleLayoutDirection(HWND hwnd) -> void
    {
        LONG exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        if (exstyle & WS_EX_LAYOUTRTL)
            SetWindowLong(hwnd, GWL_EXSTYLE, exstyle & ~WS_EX_LAYOUTRTL);
        else
        if (AscAppManager::isRtlEnabled())
            SetWindowLong(hwnd, GWL_EXSTYLE, exstyle | WS_EX_LAYOUTRTL);
    }
#endif

//    auto correctWindowMinimumSize(const QRect& windowrect, const QSize& minsize) -> QSize
//    {
//        QRect _screen_size = Utils::getScreenGeometry(windowrect.topLeft());
//        QSize _window_min_size{minsize};
//        if ( _window_min_size.width() > _screen_size.size().width() || _window_min_size.height() > _screen_size.size().height() )
//            _window_min_size.scale(_screen_size.size() - QSize(50,50), Qt::KeepAspectRatio);

//        return _window_min_size;
//    }

    auto isLeftButtonPressed() -> bool {
#ifdef Q_OS_LINUX
        return check_button_state(Qt::LeftButton);
#else
        return (::GetKeyState(VK_LBUTTON) & 0x8000) != 0;
#endif
    }

    auto constructFullscreenWidget(QWidget * panelwidget) -> CFullScrWidget *
    {
#if defined(_WIN32) && (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
        QPoint pt = panelwidget->window()->mapToGlobal(panelwidget->pos());
#else
        QPoint pt = panelwidget->mapToGlobal(panelwidget->pos());
#endif

        CTabPanel * _panel = qobject_cast<CTabPanel *>(panelwidget);
        CFullScrWidget * _parent = new CFullScrWidget;
        _parent->setWindowIcon(Utils::appIcon());
        _parent->setWindowTitle(_panel->data()->title());

        QRect _scr_geometry;
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
        int _scr_count = QApplication::desktop()->screenCount();
        if ( _scr_count > 1 ) {
            int _scrNum = QApplication::desktop()->screenNumber(pt);
            if ( _panel->reporterMode() ) {
                _scr_geometry = QApplication::desktop()->screenGeometry(_scr_count - _scrNum - 1);
            } else _scr_geometry = QApplication::desktop()->screenGeometry(_scrNum);
        } else {
            _scr_geometry = QApplication::desktop()->screenGeometry(QApplication::desktop()->primaryScreen());
        }
#else
        int _scr_count = QApplication::screens().count();
        if ( _scr_count > 1 ) {
            QScreen * _screen = QApplication::screenAt(pt);
            if ( _panel->reporterMode() ) {
                int _scrNum = QApplication::screens().indexOf(_screen);
                _scr_geometry = QApplication::screens().at(_scr_count - _scrNum - 1)->geometry();
            } else _scr_geometry = _screen->geometry();
        } else {
            _scr_geometry = QApplication::primaryScreen()->geometry();
        }
#endif

        _parent->setGeometry(_scr_geometry);
        _parent->showFullScreen();

        _panel->setParent(_parent);
        _panel->show();
        _panel->setGeometry(0,0,_parent->width(),_parent->height());

        return _parent;
    }

    auto useNativeDialog() -> bool
    {
        bool use_native_dialog = true;
#ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
        use_native_dialog = InputArgs::contains(L"--native-file-dialog");
#endif
        return use_native_dialog;
    }

    auto activeWindow() -> QWidget*
    {
#ifdef _WIN32
        HWND hwnd_top = GetForegroundWindow();
        QWidget *wgt = QWidget::find((WId)hwnd_top);
        return (wgt && wgt->isWindow()) ? wgt : nullptr;
#else
        return QApplication::activeWindow();
#endif
    }

    auto currentTopWindow() -> QWidget*
    {
        QStringList wnd_list{"MainWindow", "editorWindow"};
        QWidget *wgt = activeWindow();
        if (wgt && wnd_list.contains(wgt->objectName()) && !wgt->isMinimized()
                && wgt->property("stabilized").toBool())
            return wgt;
        return nullptr;
    }

    auto defaultWindowMaximizeState() -> bool
    {
        GET_REGISTRY_USER(reg_user);
        if (reg_user.contains("position") || reg_user.childGroups().contains("EditorsGeometry"))
            return false;
        auto scr_rc = qApp->primaryScreen()->geometry();
        return (scr_rc.width() <= SCREEN_THRESHOLD_SIZE.width() || scr_rc.height() <= SCREEN_THRESHOLD_SIZE.height());
    }
}

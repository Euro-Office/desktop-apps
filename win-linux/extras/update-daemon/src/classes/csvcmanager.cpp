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

#include "csvcmanager.h"
#include <algorithm>
#include <functional>
#include <locale>
#include <vector>
#include <sstream>
#include <numeric>
#include "version.h"
#include "classes/cjson.h"
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"
#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include "platform_win/utils.h"
# include "classes/translator.h"
# include <codecvt>
# include <Windows.h>
# include <WinInet.h>
# include <shlwapi.h>
# define APP_LAUNCH_NAME  L"/DesktopEditors.exe"
# define APP_LAUNCH_NAME2 L"/editors.exe"
# define APP_HELPER       L"/editors_helper.exe"
# define DAEMON_NAME      L"/updatesvc.exe"
# define DAEMON_NAME_OLD  L"/~updatesvc.exe"
# define RESTART_BATCH    L"/svcrestart.bat"
# define UNINSTALL_LIST   L"/unins000.bin"
# define SUBFOLDER        TEXT("/" REG_APP_NAME)
# define ARCHIVE_EXT      TEXT(".zip")
# define ARCHIVE_PATTERN  TEXT("*.zip")
# define sleep(a) Sleep(a)
#else
# include "platform_linux/utils.h"
# include <unistd.h>
# include <fnmatch.h>
# include <uuid/uuid.h>
# define APP_LAUNCH_NAME  "/DesktopEditors"
# define APP_HELPER       "/editors_helper"
# define DAEMON_NAME      "/updatesvc"
# define DAEMON_NAME_OLD  "/~updatesvc"
# define SUBFOLDER        "/desktopeditors"
# define ARCHIVE_EXT      _T(".tar.xz")
# define ARCHIVE_PATTERN  _T("*.tar.xz")
# define sleep(a) usleep(a*1000)
#endif
#ifndef URL_APPCAST_UPDATES
# define URL_APPCAST_UPDATES ""
#endif
#ifndef URL_APPCAST_DEV_CHANNEL
# define URL_APPCAST_DEV_CHANNEL ""
#endif
#define UPDATE_PATH      _T("/" REG_APP_NAME "Updates")
#define BACKUP_PATH      _T("/" REG_APP_NAME "Backup")
#define SUCCES_UNPACKED  _T("/success_unpacked.txt")
#define __GLOBAL_LOCK if (m_lock) {NS_Logger::WriteLog(_T("Blocked in: ") + FUNCTION_INFO); return;} m_lock = true; \
                          NS_Logger::WriteLog(_T("Locking and further execution: ") + FUNCTION_INFO);
#define __UNLOCK m_lock = false; NS_Logger::WriteLog(_T("Unlocked in: ") + FUNCTION_INFO);

using std::vector;


auto currentArch()->tstring
{
#ifdef _WIN32
# if defined(_M_ARM64)
    return L"_arm64";
# elif defined(_M_X64)
    return L"_x64";
# elif defined(_M_IX86)
    return L"_x86";
# endif
#else
    return _T("_x64");
#endif
}

auto generateTmpFileName(const tstring &ext)->tstring
{
    tstring uuid_tstr;
#ifdef _WIN32
    UUID uuid = {0};
    RPC_WSTR wszUuid = NULL;
    if (UuidCreate(&uuid) == RPC_S_OK && UuidToStringW(&uuid, &wszUuid) == RPC_S_OK) {
        uuid_tstr = ((wchar_t*)wszUuid);
        RpcStringFreeW(&wszUuid);
    } else
        uuid_tstr = L"00000000-0000-0000-0000-000000000000";
#else
    uuid_t uuid;
    char uuid_str[37];
    uuid_generate(uuid);
    uuid_unparse(uuid, uuid_str);
    uuid_tstr = uuid_str;
#endif
    return NS_File::tempPath() + _T("/") + _T(FILE_PREFIX) + uuid_tstr + currentArch() + ext;
}

auto isSuccessUnpacked(const tstring &successFilePath, const tstring &version)->bool
{
    list<tstring> lines;
    if (NS_File::readFile(successFilePath, lines)) {
        if (std::find(lines.begin(), lines.end(), version) != lines.end())
            return true;
    }
    return false;
}

auto isVersionBHigherThanA(const tstring &a, const tstring &b)->bool {
    tstringstream old_ver(a), new_ver(b);
    tstring old_part, new_part;
    while (std::getline(old_ver, old_part, _T('.')) && std::getline(new_ver, new_part, _T('.'))) {
        int old_num = 0, new_num = 0;
        try {
            old_num = std::stoi(old_part);
        } catch (...) {};
        try {
            new_num = std::stoi(new_part);
        } catch (...) {};
        if (new_num > old_num)
            return true;
        else
        if (new_num < old_num)
            return false;
    }
    return false;
}

auto replace(tstring &str, const tstring &from, const tstring &to)->void {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != tstring::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

#ifdef _WIN32
auto restartService()->void
{
    wstring fileName = NS_File::appPath() + RESTART_BATCH;
    if (NS_File::fileExists(fileName) && !NS_File::removeFile(fileName)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR2) + _T(" ") + fileName, true);
        return;
    }

    wstring args = NS_Utils::cmdArgsAsString();
    if (!args.empty())
        args.insert(0, 1, L' ');

    std::list<wstring> batch = {
        L"@chcp 65001>nul",
        L"@echo off",
        wstring(L"NET STOP ") + L"\"" + TEXT(VER_PRODUCTNAME_STR) + L"\"",
        wstring(L"SC START ") + L"\"" +  TEXT(VER_PRODUCTNAME_STR) + L"\"" + args,
        L"del /F /Q \"%~dp0~updatesvc.exe\"",
        L"exit"
    };

    if (!NS_File::writeToFile(fileName, batch)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR3) + _T(" ") + fileName, true);
        return;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    if (!CreateProcess(NULL, &fileName[0], NULL, NULL, FALSE,
                          CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi))
    {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR4), true);
        return;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

auto verToAppVer(const wstring &ver)->wstring
{
    size_t pos = ver.find(L'.');
    if (pos == std::wstring::npos)
        return ver;

    pos = ver.find(L'.', pos + 1);
    if (pos == std::wstring::npos)
        return ver;

    pos = ver.find(L'.', pos + 1);
    return (pos == std::wstring::npos) ? ver : ver.substr(0, pos);
}

auto displayNameReplaceVersion(const wstring &disp_name, const wstring &newVersion)->wstring
{
    for (size_t i = disp_name.size(); i-- > 0;) {
        if (iswdigit(disp_name[i])) {
            size_t j = i;
            bool hasDot = false;
            while (j > 0 && (iswdigit(disp_name[j - 1]) || disp_name[j - 1] == L'.')) {
                if (disp_name[j - 1] == L'.')
                    hasDot = true;
                --j;
            }
            if (hasDot) {
                return disp_name.substr(0, j) + newVersion + disp_name.substr(i + 1);
            } else {
                i = j;
            }
        }
    }
    return disp_name;
}

auto getCurrentDate()->wstring
{
    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    wchar_t frmDate[9] = {0};
    swprintf(frmDate, _ARRAYSIZE(frmDate), L"%04d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);
    return frmDate;
}
#endif

struct CSvcManager::PackageData {
    tstring fileName,
            fileType,
            object,
            hash,
            version,
            packageUrl,
            packageArgs;
    bool    isInstallable = true;
    void clear() {
        fileName.clear();
        fileType.clear();
        object.clear();
        hash.clear();
        version.clear();
        packageUrl.clear();
        packageArgs.clear();
        isInstallable = true;
    }
};

struct CSvcManager::SavedPackageData {
    tstring fileName;
};

CSvcManager::CSvcManager():
    m_packageData(new PackageData),
    m_savedPackageData(new SavedPackageData),
    m_downloadMode(Mode::CHECK_UPDATES),
    m_packageType(Package::Portable),
    m_socket(new CSocket(APP_PORT, SVC_PORT)),
    m_pDownloader(new CDownloader),
    m_pUnzip(new CUnzip)
{
    m_checkUrl = NS_Utils::cmdArgContains(_T("--appcast-dev-channel")) ? _T(URL_APPCAST_DEV_CHANNEL) : _T(URL_APPCAST_UPDATES);
    NS_Logger::WriteLog(m_checkUrl.empty() ? _T("Updates is off, URL is empty.") : _T("Updates is on, URL: ") + m_checkUrl);
    init();
}

CSvcManager::~CSvcManager()
{
    if (m_future_clear.valid())
        m_future_clear.wait();
    delete m_packageData, m_packageData = nullptr;
    delete m_savedPackageData, m_savedPackageData = nullptr;
    delete m_pUnzip, m_pUnzip = nullptr;
    delete m_pDownloader, m_pDownloader = nullptr;
    delete m_socket, m_socket = nullptr;
    if (m_quit_callback)
        m_quit_callback();
}

void CSvcManager::aboutToQuit(FnVoidVoid callback)
{
    m_quit_callback = callback;
}

void CSvcManager::init()
{
    m_pDownloader->onQueryResponse([=](int error, int lenght) {
        onQueryResponse(error, lenght);
    });
    m_pDownloader->onComplete([=](int error) {
        onCompleteSlot(error, m_pDownloader->GetFilePath());
    });
    m_pDownloader->onProgress([=](int percent) {
        onProgressSlot(percent);
    });
    m_pUnzip->onComplete([=](int error) {
        onCompleteUnzip(error);
    });
    m_pUnzip->onProgress([=](int percent) {
        m_socket->sendMessage(MSG_UnzipProgress, to_tstring(percent));
    });
    m_socket->onMessageReceived([=](void *data, size_t) {
        vector<tstring> params;
        if (m_socket->parseMessage(data, params) == 3) {
            switch (std::stoi(params[0])) {
            case MSG_CheckUpdates: {
                __GLOBAL_LOCK
                //DeleteUrlCacheEntry(params[1].c_str());
                m_packageData->clear();
                if (!m_checkUrl.empty()) {
                    JsonDocument doc(params[1]);
                    JsonObject root = doc.object();
                    m_currVersion = root.value(_T("currVersion")).toTString();
                    m_ignVersion = root.value(_T("ignVersion")).toTString();
                    m_savedPackageData->fileName = root.value(_T("fileName")).toTString();
                    tstring package_str = root.value(_T("package")).toTString();
                    m_packageType = package_str == _T("iss") ? ISS : package_str == _T("msi") ? MSI : package_str == _T("portable") ? Portable : Other;
                    m_downloadMode = Mode::CHECK_UPDATES;
                    if (m_pDownloader)
                        m_pDownloader->downloadFile(m_checkUrl, generateTmpFileName(_T(".json")));
                    NS_Logger::WriteLog(_T("Received MSG_CheckUpdates, URL: ") + m_checkUrl);
                } else {
                    m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_URL"));
                    __UNLOCK
                }
                break;
            }
            case MSG_LoadUpdates: {
                __GLOBAL_LOCK
                m_downloadMode = Mode::DOWNLOAD_UPDATES;
                if (m_pDownloader) {
                    tstring ext = m_packageData->fileType == _T("iss") ? _T(".exe") : m_packageData->fileType == _T("msi") ? _T(".msi") : ARCHIVE_EXT;
                    m_pDownloader->downloadFile(m_packageData->packageUrl, generateTmpFileName(ext));
                }
                NS_Logger::WriteLog(_T("Received MSG_LoadUpdates, URL: ") + m_packageData->packageUrl);
                break;
            }
            case MSG_RequestContentLenght: {
                __GLOBAL_LOCK
                if (m_pDownloader)
                    m_pDownloader->queryContentLenght(m_packageData->packageUrl);
                NS_Logger::WriteLog(_T("Received MSG_RequestContentLenght, URL: ") + m_packageData->packageUrl);
                break;
            }
            case MSG_StopDownload: {
                m_downloadMode = Mode::CHECK_UPDATES;
                if (m_pDownloader)
                    m_pDownloader->stop();
                break;
            }
            case MSG_UnzipIfNeeded:
                if (!m_packageData->fileName.empty() && NS_File::getFileHash(m_packageData->fileName) == m_packageData->hash) {
                    unzipIfNeeded(m_packageData->fileName, m_packageData->version);
                } else {
                    m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_MD5"));
                }
                break;

            case MSG_StartReplacingFiles:
                __GLOBAL_LOCK
                startReplacingFiles(params[1], params[2] == _T("true"));
                __UNLOCK
                break;

            case MSG_StartReplacingService:
                __GLOBAL_LOCK
                startReplacingService(params[2] == _T("true"));
                __UNLOCK
                break;
#ifdef _WIN32
            case MSG_StartInstallPackage:
                if (!m_packageData->fileName.empty() && NS_File::getFileHash(m_packageData->fileName) == m_packageData->hash) {
                    __GLOBAL_LOCK
                    startInstallPackage();
                    __UNLOCK
                } else {
                    m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_MD5"));
                }
                break;
#endif
            case MSG_ClearTempFiles:
                clearTempFiles(params[1], params[2]);
                break;

            case MSG_SetLanguage:
                Translator::instance().setLanguage(params[1]);
                break;

            default:
                break;
            }
        }
    });

    m_socket->onError([](const char* error) {
        tstring _error;
#ifdef _WIN32
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        _error = converter.from_bytes(error);
#else
        _error = error;
#endif
        NS_Logger::WriteLog(_error);
    });
}

void CSvcManager::onQueryResponse(const int error, const int lenght)
{
    __UNLOCK
    m_socket->sendMessage(MSG_RequestContentLenght, (error == 0) ? to_tstring(lenght) : _T(""));
}

void CSvcManager::onCompleteUnzip(const int error)
{
    __UNLOCK
    if (error == UNZIP_OK) {
        // Сreate a file about successful unpacking for use in subsequent launches
        const tstring updPath = NS_File::parentPath(NS_File::appPath()) + UPDATE_PATH;
        list<tstring> successList{m_newVersion};
        if (!NS_File::writeToFile(updPath + SUCCES_UNPACKED, successList)) {
            return;
        }
#ifdef _WIN32
        // Adding new app files to the uninstall list
        auto fillSubpathVec = [](const tstring &path, vector<tstring> &vec)->bool {
            tstring _error;
            list<tstring> filesList;
            if (!NS_File::GetFilesList(path, &filesList, _error)) {
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE + TEXT(" ") + _error);
                return false;
            }
            for (auto &filePath : filesList) {
                tstring subPath = filePath.substr(path.length());
                vec.push_back(std::move(subPath));
            }
            return true;
        };

        vector<wstring> updVec, appVec;
        const tstring appPath = NS_File::appPath();
        if (fillSubpathVec(appPath, appVec) && fillSubpathVec(updPath, updVec)) {
            list<tstring> delList;
            if (NS_File::fileExists(appPath + UNINSTALL_LIST) && !NS_File::readBinFile(appPath + UNINSTALL_LIST, delList))
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);

            for (auto &updFile : updVec) {
                if (std::find(appVec.begin(), appVec.end(), updFile) == appVec.end())
                    delList.push_back(NS_File::toNativeSeparators(updFile));
            }
            if (!NS_File::writeToBinFile(updPath + UNINSTALL_LIST, delList))
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
        }
#endif
        if (!m_socket->sendMessage(MSG_ShowStartInstallMessage))
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);

    } else
    if (error == UNZIP_ERROR) {
        if (!m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_UNPACKING")))
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);

    } else
    if (error == UNZIP_ABORT) {
        // Stop unzip
    }
}

void CSvcManager::onCompleteSlot(const int error, const tstring &filePath)
{
    __UNLOCK
    if (error == 0) {
        switch (m_downloadMode) {
        case Mode::CHECK_UPDATES: {
            __GLOBAL_LOCK // isUrlAccessible may take a long time to execute
            tstring out_json;
            list<tstring> lst;
            if (NS_File::readFile(filePath, lst)) {
                tstring json = std::accumulate(lst.begin(), lst.end(), tstring());
                JsonDocument doc(json);
                JsonObject root = doc.object();

                tstring version = root.value(_T("version")).toTString();
                tstring curr_version = m_currVersion;
                tstring svc_version = root.value(_T("serviceVersion")).toTString();
                tstring curr_svc_version = _T(VER_FILEVERSION_STR);
                JsonObject package = root.value(_T("package")).toObject();
#ifdef _WIN32
# if defined(_M_ARM64)
                JsonObject win = package.value(_T("win_arm64")).toObject();
# elif defined(_M_X64)
                JsonObject win = package.value(_T("win_64")).toObject();
# elif defined(_M_IX86)
                JsonObject win = package.value(_T("win_32")).toObject();
# endif
#else
                JsonObject win = package.value(_T("linux_64")).toObject();
#endif
                if (isVersionBHigherThanA(curr_version, version) && (version != m_ignVersion)) {
                    m_packageData->object = _T("app");
                    m_packageData->version = version;
                    m_packageData->fileType = _T("archive");
                    JsonObject package_type = win.value(_T("archive")).toObject();
#ifdef _WIN32
                    if (m_packageType != Portable) {
                        const tstring install_key = m_packageType == MSI ? _T("msi") : _T("iss");
                        if (win.contains(install_key)) {
                            JsonObject install_type = win.value(install_key).toObject();
                            if (install_type.contains(_T("maxVersion"))) {
                                tstring maxVersion = install_type.value(_T("maxVersion")).toTString();
                                if (!isVersionBHigherThanA(maxVersion, curr_version)) {
                                    package_type = install_type;
                                    m_packageData->fileType = install_key;
                                    m_packageData->packageArgs = package_type.value(_T("arguments")).toTString();
                                }
                            }
                        }
                    }
#endif
                    tstring url = package_type.value(_T("url")).toTString();
                    tstring url2 = package_type.value(_T("url2")).toTString();
                    NS_Logger::WriteLog(_T("Primary package URL: ") + url + _T("\nSecondary package URL: ") + url2);
                    m_packageData->packageUrl = ((url.empty() || !m_pDownloader->isUrlAccessible(url)) && !url2.empty()) ? url2 : url;
                    tstring hash = package_type.value(_T("md5")).toTString();
                    std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
                    m_packageData->hash = hash;

                    // parse release notes
                    // JsonObject release_notes = root.value(_T("releaseNotes")).toObject();
                    // const tstring lang = CLangater::getCurrentLangCode() == "ru-RU" ? "ru-RU" : "en-EN";
                    // JsonValue changelog = release_notes.value(lang);

                    tstring min_version = root.value(_T("minVersion")).toTString();
                    if (!min_version.empty() && isVersionBHigherThanA(curr_version, min_version))
                        m_packageData->isInstallable = false;

                } else
                if (isVersionBHigherThanA(curr_svc_version, svc_version)) {
                    m_packageData->object = _T("svc");
                    m_packageData->version = svc_version;
                    m_packageData->fileType = _T("archive");
                    JsonObject package_type = win.value(_T("serviceArchive")).toObject();
                    tstring url = package_type.value(_T("url")).toTString();
                    tstring url2 = package_type.value(_T("url2")).toTString();
                    NS_Logger::WriteLog(_T("Primary package URL: ") + url + _T("\nSecondary package URL: ") + url2);
                    m_packageData->packageUrl = ((url.empty() || !m_pDownloader->isUrlAccessible(url)) && !url2.empty()) ? url2 : url;
                    tstring hash = package_type.value(_T("md5")).toTString();
                    std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
                    m_packageData->hash = hash;

                } else {
                    out_json = _T("{}");
                }

                if (out_json.empty()) {
                    out_json = _T("{\"object\":\"%1\",\"version\":\"%2\",\"fileType\":\"%3\",\"packageUrl\":\"%4\",\"packageArgs\":\"%5\","
                                  "\"hash\":\"%6\",\"isInstallable\":%7}");
                    replace(out_json, _T("%1"), m_packageData->object);
                    replace(out_json, _T("%2"), m_packageData->version);
                    replace(out_json, _T("%3"), m_packageData->fileType);
                    replace(out_json, _T("%4"), m_packageData->packageUrl);
                    replace(out_json, _T("%5"), m_packageData->packageArgs);
                    replace(out_json, _T("%6"), m_packageData->hash);
                    replace(out_json, _T("%7"), m_packageData->isInstallable ? _T("true") : _T("false"));
                    if (!m_savedPackageData->fileName.empty() && NS_File::getFileHash(m_savedPackageData->fileName) == m_packageData->hash)
                        m_packageData->fileName = m_savedPackageData->fileName;
                }

            } else {
                // read error
            }
            __UNLOCK
            m_socket->sendMessage(MSG_LoadCheckFinished, out_json);
            break;
        }
        case Mode::DOWNLOAD_UPDATES:
            if (!filePath.empty() && NS_File::getFileHash(filePath) == m_packageData->hash) {
                m_packageData->fileName = filePath;
                m_socket->sendMessage(MSG_LoadUpdateFinished, filePath);
            } else {
                m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_MD5"));
            }
            break;
        default:
            break;
        }
    } else
    if (error == 1) {
        // Pause or Stop
    } else
    if (error == -1) {
        m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_DNL_OUT_MEM"));
    } else
    if (error == -2) {
        m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_DNL_CONN"));
    } else
    if (error == -3) {
        m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_DNL_URL"));
    } else
    if (error == -4) {
        m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_DNL_CREAT"));
    } else {
        m_socket->sendMessage(MSG_OtherError, _T("SVC_TXT_ERR_DNL_INET"));
    }
}

void CSvcManager::onProgressSlot(const int percent)
{
    if (m_downloadMode == Mode::DOWNLOAD_UPDATES)
        m_socket->sendMessage(MSG_Progress, to_tstring(percent));
}

void CSvcManager::unzipIfNeeded(const tstring &filePath, const tstring &newVersion)
{
    __GLOBAL_LOCK

    m_newVersion = newVersion;
    const tstring updPath = NS_File::parentPath(NS_File::appPath()) + UPDATE_PATH;
    auto unzip = [=]()->void {
        if (!NS_File::dirExists(updPath) && !NS_File::makePath(updPath)) {
            NS_Logger::WriteLog(_T("An error occurred while creating dir: ") + updPath);
            __UNLOCK
            return;
        }
        m_pUnzip->extractArchive(filePath, updPath);
    };

    if (!NS_File::dirExists(updPath) || NS_File::dirIsEmpty(updPath)) {
        unzip();
    } else {
        if (isSuccessUnpacked(updPath + SUCCES_UNPACKED, newVersion)) {
            __UNLOCK
            if (!m_socket->sendMessage(MSG_ShowStartInstallMessage))
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);

        } else {
            if (!NS_File::removeDirRecursively(updPath))
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            unzip();
        }
    }
}

void CSvcManager::clearTempFiles(const tstring &prefix, const tstring &except)
{
    m_future_clear = std::async(std::launch::async, [=]() {
#ifdef _WIN32
        if (NS_File::fileExists(NS_File::appPath() + RESTART_BATCH))
            NS_File::removeFile(NS_File::appPath() + RESTART_BATCH);
#endif
        tstring _error;
        list<tstring> filesList;
        if (!NS_File::GetFilesList(NS_File::tempPath(), &filesList, _error, true)) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE + _T(" ") + _error);
            return;
        }
        for (auto &filePath : filesList) {
#ifdef _WIN32
            if (PathMatchSpec(filePath.c_str(), L"*.json") || PathMatchSpec(filePath.c_str(), ARCHIVE_PATTERN)
                || PathMatchSpec(filePath.c_str(), L"*.msi") || PathMatchSpec(filePath.c_str(), L"*.exe")) {
#else
            if (fnmatch("*.json", filePath.c_str(), 0) == 0 || fnmatch(ARCHIVE_PATTERN, filePath.c_str(), 0) == 0) {
#endif
                tstring lcFilePath(filePath);
                std::transform(lcFilePath.begin(), lcFilePath.end(), lcFilePath.begin(), ::tolower);
                if (lcFilePath.find(prefix) != tstring::npos && filePath != except)
#ifdef _WIN32
                    NS_File::removeFile(filePath, true);
#else
                    NS_File::removeFile(filePath);
#endif
            }
        }
        tstring updPath = NS_File::parentPath(NS_File::appPath()) + UPDATE_PATH;
        if (except.empty() && NS_File::dirExists(updPath))
            NS_File::removeDirRecursively(updPath);
    });
}

void CSvcManager::startReplacingFiles(const tstring &packageType, const bool restartAfterUpdate)
{
    tstring appPath = NS_File::appPath();
    tstring updPath = NS_File::parentPath(appPath) + UPDATE_PATH;
    tstring updSubPath = NS_File::fileExists(updPath + SUBFOLDER + APP_LAUNCH_NAME) ? updPath + SUBFOLDER : updPath;
    tstring tmpPath = NS_File::parentPath(appPath) + BACKUP_PATH;
    if (!NS_File::dirExists(updPath)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR5) + _T(" ") + updPath, true);
        return;
    }

#ifdef _WIN32
# ifndef DONT_VERIFY_SIGNATURE
    // Verify the signature of executable files
    {
        tstring apps[] = {APP_LAUNCH_NAME, APP_LAUNCH_NAME2, APP_HELPER, DAEMON_NAME};
        for (int i = 0; i < sizeof(apps) / sizeof(apps[0]); i++) {
            if (!NS_File::verifyEmbeddedSignature(updSubPath + apps[i])) {
                NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR6) + _T(" ") + updSubPath + apps[i], true);
                return;
            }
        }
    }
# endif
#endif

    // Check backup folder
    if (NS_File::dirExists(tmpPath) && !NS_File::removeDirRecursively(tmpPath)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR7) + _T(" ") + tmpPath, true);
        return;
    }

    // Wait until the main app closes
    {
#ifdef _WIN32
        tstring apps[] = {APP_LAUNCH_NAME2, APP_HELPER};
#else
        tstring apps[] = {APP_LAUNCH_NAME, APP_HELPER};
#endif
        for (int i = 0; i < sizeof(apps) / sizeof(apps[0]); i++) {
            int retries = 10;
#ifdef _WIN32
            tstring app = NS_File::toNativeSeparators(appPath + apps[i]);
#else
            tstring app(apps[i]);
            app = app.substr(1);
#endif
            while (NS_File::isProcessRunning(app) && retries-- > 0)
                sleep(500);

            if (NS_File::isProcessRunning(app)) {
                NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR8) + _T(" ") + app, true);
                return;
            }
        }
    }

    // Replace app path to Backup
#ifdef _WIN32_UNUSED
    if (packageType == TEXT("portable") && !NS_File::dirExists(tmpPath) && !NS_File::makePath(tmpPath)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR9) + _T(" ") + tmpPath, true);
        return;
    }
    if (!NS_File::replaceFolder(appPath, tmpPath, packageType != TEXT("portable"))) {
#else
    if (!NS_File::replaceFolder(appPath, tmpPath, true)) {
#endif
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR10) + _T(" ") + NS_Utils::GetLastErrorAsString(), true);
        if (NS_File::dirExists(tmpPath) && !NS_File::dirIsEmpty(tmpPath) && !NS_File::replaceFolder(tmpPath, appPath))
            NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR11), true);
        return;
    }

    // Move update path to app path
    if (!NS_File::replaceFolder(updSubPath, appPath, true)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR12) + _T(" ") + NS_Utils::GetLastErrorAsString(), true);

        if (NS_File::dirExists(appPath) && !NS_File::removeDirRecursively(appPath)) {
            NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR13) + _T(" ") + NS_Utils::GetLastErrorAsString(), true);
            return;
        }
        if (!NS_File::replaceFolder(tmpPath, appPath, true))
            NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR14) + _T(" ") + NS_Utils::GetLastErrorAsString(), true);

        NS_File::removeDirRecursively(updPath);
        return;
    }

#ifdef _WIN32
    // Moving the necessary files to their original location
    {
        tstring files[] = {L"/unins000.msg", L"/unins000.dat", L"/unins000.exe", L"/converter/package.config"};
        for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
            if (NS_File::fileExists(tmpPath + files[i]))
                NS_File::replaceFile(tmpPath + files[i], appPath + files[i]);
        }

        auto licenseFiles = NS_File::findFilesByPattern(tmpPath, L"LICENSE.*");
        auto eulaFiles = NS_File::findFilesByPattern(tmpPath, L"EULA.*");
        licenseFiles.insert(licenseFiles.end(), eulaFiles.begin(), eulaFiles.end());
        for (const auto &file : licenseFiles) {
            if (!NS_File::fileExists(appPath + file) && NS_File::fileExists(tmpPath + file))
                NS_File::replaceFile(tmpPath + file, appPath + file);
        }
    }

    // To support a version without updatesvc.exe inside the working folder
    if (!NS_File::fileExists(appPath + DAEMON_NAME))
        NS_File::replaceFile(tmpPath + DAEMON_NAME, appPath + DAEMON_NAME);
    else
        NS_File::replaceFile(tmpPath + DAEMON_NAME, appPath + DAEMON_NAME_OLD);

    // Update version in registry
    if (packageType == TEXT("iss") || packageType == TEXT("msi")) {
        wstring ver;
        list<wstring> lines;
        if (NS_File::readFile(appPath + SUCCES_UNPACKED, lines)) {
            if (lines.size() > 0)
                ver = lines.front();
            NS_File::removeFile(appPath + SUCCES_UNPACKED);
        }
        if (!ver.empty()) {
            HKEY hKey, hAppKey;
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
                wstring app_name(TEXT(WINDOW_NAME));
                wstring app_key(TEXT(REG_UNINST_KEY));
                app_key += (packageType == TEXT("iss")) ? L"_is1" : L"";
                if (RegOpenKeyEx(hKey, app_key.c_str(), 0, KEY_ALL_ACCESS, &hAppKey) == ERROR_SUCCESS) {
                    wstring disp_name;
                    wstring ins_date = getCurrentDate();
                    {
                        WCHAR pData[MAX_PATH] = {};
                        DWORD cbData = sizeof(pData);
                        DWORD dwType = REG_SZ;
                        if (RegQueryValueEx(hAppKey, TEXT("DisplayName"), nullptr, &dwType, (LPBYTE)pData, &cbData) == ERROR_SUCCESS && pData[0] != L'\0') {
                            disp_name = displayNameReplaceVersion(pData, verToAppVer(ver));
                        } else {
                            NS_Logger::WriteLog(L"Unable to get DisplayName from registry!");
                            disp_name = app_name + L" " + verToAppVer(ver) + L" (" + currentArch().substr(1) + L")";
                        }
                    }
                    if (RegSetValueEx(hAppKey, TEXT("DisplayName"), 0, REG_SZ, (const BYTE*)disp_name.c_str(), (DWORD)(disp_name.length() + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
                        NS_Logger::WriteLog(L"Can't update DisplayName in registry!");
                    if (RegSetValueEx(hAppKey, TEXT("DisplayVersion"), 0, REG_SZ, (const BYTE*)ver.c_str(), (DWORD)(ver.length() + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
                        NS_Logger::WriteLog(L"Can't update DisplayVersion in registry!");
                    if (RegSetValueEx(hAppKey, TEXT("InstallDate"), 0, REG_SZ, (const BYTE*)ins_date.c_str(), (DWORD)(ins_date.length() + 1) * sizeof(WCHAR)) != ERROR_SUCCESS)
                        NS_Logger::WriteLog(L"Can't update InstallDate in registry!");
                    RegCloseKey(hAppKey);
                }
                RegCloseKey(hKey);
            }
        }
    }
#endif

    // Merging providers, templates, uithemes folders
    {
        tstring paths[] = {_T("/providers"), _T("/converter/empty"), _T("/uithemes")};
        for (int i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
            tstring err;
            std::list<tstring> old_paths, new_paths;
            if (NS_File::GetFilesList(tmpPath + paths[i], &old_paths, err, true, true)) {
                if (NS_File::GetFilesList(appPath + paths[i], &new_paths, err, true, true)) {
                    for (auto &path : old_paths) {
                        if (std::find(new_paths.begin(), new_paths.end(), path) == new_paths.end())
                            NS_File::replaceFolder(tmpPath + paths[i] + path, appPath + paths[i] + path);
                    }
                } else
                    NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE + _T(" ") + err);
            } else
                NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE + _T(" ") + err);
        }
    }

    // Restart program
    if (restartAfterUpdate) {
        if (!NS_File::runProcess(appPath + APP_LAUNCH_NAME, _T("")))
            NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR15), true);
    }

    // Remove Backup dir
    NS_File::removeDirRecursively(tmpPath);

    // Restart service
#ifdef _WIN32
    restartService();
#endif
}

void CSvcManager::startReplacingService(const bool restartAfterUpdate)
{
    tstring appPath = NS_File::appPath();
    tstring updPath = NS_File::parentPath(appPath) + UPDATE_PATH;
    tstring updSubPath = NS_File::fileExists(updPath + SUBFOLDER + APP_LAUNCH_NAME) ? updPath + SUBFOLDER : updPath;
    if (!NS_File::dirExists(updPath)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR5) + _T(" ") + updPath, true);
        return;
    }

#ifdef _WIN32
# ifndef DONT_VERIFY_SIGNATURE
    // Verify the signature of executable files
    if (!NS_File::verifyEmbeddedSignature(updSubPath + DAEMON_NAME)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR6) + _T(" ") + updSubPath + DAEMON_NAME, true);
        return;
    }
# endif
#endif

    // Wait until the main app closes
    {
#ifdef _WIN32
        tstring apps[] = {APP_LAUNCH_NAME2, APP_HELPER};
#else
        tstring apps[] = {APP_LAUNCH_NAME, APP_HELPER};
#endif
        for (int i = 0; i < sizeof(apps) / sizeof(apps[0]); i++) {
            int retries = 10;
#ifdef _WIN32
            tstring app = NS_File::toNativeSeparators(appPath + apps[i]);
#else
            tstring app(apps[i]);
            app = app.substr(1);
#endif
            while (NS_File::isProcessRunning(app) && retries-- > 0)
                sleep(500);

            if (NS_File::isProcessRunning(app)) {
                NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR8) + _T(" ") + app, true);
                return;
            }
        }
    }

    // Rename updatesvc.exe to ~updatesvc.exe
    if (NS_File::fileExists(appPath + DAEMON_NAME) && !NS_File::replaceFile(appPath + DAEMON_NAME, appPath + DAEMON_NAME_OLD)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR19) + _T(" ") + NS_Utils::GetLastErrorAsString(), true);
        return;
    }

    // Move updatesvc.exe to app path
    if (!NS_File::replaceFile(updSubPath + DAEMON_NAME, appPath + DAEMON_NAME)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR20) + _T(" ") + NS_Utils::GetLastErrorAsString(), true);
        if (NS_File::fileExists(appPath + DAEMON_NAME_OLD) && !NS_File::replaceFile(appPath + DAEMON_NAME_OLD, appPath + DAEMON_NAME))
            NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR21), true);
        return;
    }

    // Restart program
    if (restartAfterUpdate) {
        if (!NS_File::runProcess(appPath + APP_LAUNCH_NAME, _T("")))
            NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR15), true);
    }

    // Remove Update dir
    NS_File::removeDirRecursively(updPath);

    // Restart service
#ifdef _WIN32
    restartService();
#endif
}

#ifdef _WIN32
void CSvcManager::startInstallPackage()
{
    // Verify the signature of executable files
    if (!NS_File::verifyEmbeddedSignature(m_packageData->fileName)) {
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR6) + _T(" ") + m_packageData->fileName, true);
        return;
    }
    tstring args;
    if (m_packageData->fileType == _T("msi")) {
        args = _T("/i \"") + NS_File::toNativeSeparators(m_packageData->fileName) + _T("\"");
        if (!m_packageData->packageArgs.empty())
            args += _T(" ") + m_packageData->packageArgs;
    } else {
        args = m_packageData->packageArgs;
        if (!args.empty())
            args += _T(" ");
        args += _T("/LANG=") + NS_Utils::GetAppLanguage();
    }
    if (!NS_File::runProcess(m_packageData->fileType == _T("msi") ? _T("msiexec.exe") : m_packageData->fileName, args))
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR18), true);
}
#endif

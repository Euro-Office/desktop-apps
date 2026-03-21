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

#include "platform_win/utils.h"
#include "classes/translator.h"
#include "version.h"
#include <Windows.h>
#include <shlwapi.h>
#include <ShlObj.h>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cstdio>
#include <Wincrypt.h>
#include <WtsApi32.h>
#include <Softpub.h>
#include <TlHelp32.h>
#include <userenv.h>
#include <vector>
#include <sstream>
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"

#define BUFSIZE 1024


static DWORD GetActiveSessionId()
{
    DWORD sesId = MAXDWORD, count = 0;
    WTS_SESSION_INFO *sesInfo = NULL;
    if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sesInfo, &count)) {
        for (DWORD i = 0; i < count; i++) {
            if (sesInfo[i].State == WTSActive) {
                sesId = sesInfo[i].SessionId;
                break;
            }
        }
        WTSFreeMemory(sesInfo);
    }
    return sesId;
}

static bool GetDuplicateToken(HANDLE &hTokenDup)
{
    DWORD sesId = GetActiveSessionId();
    if (sesId == 0xFFFFFFFF) {
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        return false;
    }
    HANDLE hUserToken = NULL;
    if (!WTSQueryUserToken(sesId, &hUserToken)) {
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        return false;
    }
    if (!DuplicateTokenEx(hUserToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hTokenDup)) {
        CloseHandle(hUserToken);
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        return false;
    }
    CloseHandle(hUserToken);
    return true;
}

static HRESULT PerformFileOperation(const wstring &pFrom, const std::function<HRESULT(IFileOperation *pfo, IShellItem *pSrcItem)> &callback)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOperation *pfo;
        hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
        if (SUCCEEDED(hr)) {
            hr = pfo->SetOperationFlags(FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SILENT);
            if (SUCCEEDED(hr)) {
                IShellItem *pSrcItem;
                hr = SHCreateItemFromParsingName(pFrom.c_str(), NULL, IID_PPV_ARGS(&pSrcItem));
                if (SUCCEEDED(hr)) {
                    hr = callback(pfo, pSrcItem);
                    pSrcItem->Release();
                }
            }
            pfo->Release();
        }
        CoUninitialize();
    }
    return hr;
}

namespace NS_Utils
{
    bool run_as_app = false;

    void setRunAsApp()
    {
        run_as_app = true;
    }

    bool isRunAsApp()
    {
        return run_as_app;
    }

    std::vector<wstring> cmd_args;

    void parseCmdArgs(int argc, wchar_t *argv[])
    {
        for (int i = 0; i < argc; i++)
            cmd_args.push_back(argv[i]);
    }

    bool cmdArgContains(const wstring &param)
    {
        auto len = param.length();
        return std::any_of(cmd_args.cbegin(), cmd_args.cend(), [&param, len](const wstring &arg) {
            return arg.find(param) == 0 && (len == arg.length() || arg[len] == L'=' || arg[len] == L':' || arg[len] == L'|');
        });
    }

    wstring cmdArgValue(const wstring &param)
    {
        auto len = param.length();
        for (const auto &arg : cmd_args) {
            if (arg.find(param) == 0 && len < arg.length() && (arg[len] == L'=' || arg[len] == L':' || arg[len] == L'|'))
                return arg.substr(len + 1);
        }
        return L"";
    }

    wstring cmdArgsAsString()
    {
        if (cmd_args.empty())
            return L"";
        wstring args = cmd_args[0];
        for (size_t i = 1; i < cmd_args.size(); ++i) {
            args += L" " + cmd_args[i];
        }
        return args;
    }

    wstring GetLastErrorAsString()
    {
        DWORD errID = ::GetLastError();
        if (errID == 0)
            return L"";

        LPWSTR msgBuff = NULL;
        size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                       NULL, errID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgBuff, 0, NULL);
        wstring msg;
        if (size > 0) {
            msg.assign(msgBuff, size);
            LocalFree(msgBuff);
        }
        return msg;
    }

    int ShowMessage(wstring str, bool showError)
    {
        if (showError)
            str += L" " + GetLastErrorAsString();
        wstring prod_name = _TR(CAPTION_TEXT);
        wchar_t *title = const_cast<LPTSTR>(prod_name.c_str());
        if (isRunAsApp()) {
            MessageBox(NULL, str.c_str(), title, MB_ICONERROR | MB_SERVICE_NOTIFICATION_NT3X | MB_SETFOREGROUND);
            return 0;
        }
        DWORD title_size = (DWORD)wcslen(title) * sizeof(wchar_t);
        DWORD res;
        DWORD session_id = GetActiveSessionId();
        WTSSendMessageW(WTS_CURRENT_SERVER_HANDLE, session_id, title, title_size,
                            const_cast<LPTSTR>(str.c_str()), (DWORD)str.size() * sizeof(wchar_t),
                            MB_OK | MB_ICONERROR | MB_SERVICE_NOTIFICATION_NT3X | MB_SETFOREGROUND, 30, &res, TRUE);
        return res;
    }

    wstring GetAppLanguage()
    {
        wstring lang = TEXT("en_US"), subkey = TEXT("SOFTWARE\\" REG_GROUP_KEY "\\" REG_APP_NAME);
        HKEY hKey = NULL, hRootKey = isRunAsApp() ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
        if (RegOpenKeyEx(hRootKey, subkey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD type = REG_SZ, cbData = 0;
            if (RegGetValue(hKey, NULL, TEXT("locale"), RRF_RT_REG_SZ, &type, NULL, &cbData) == ERROR_SUCCESS) {
                wchar_t *pvData = (wchar_t*)malloc(cbData);
                if (RegGetValueW(hKey, NULL, TEXT("locale"), RRF_RT_REG_SZ, &type, (void*)pvData, &cbData) == ERROR_SUCCESS)
                    lang = pvData;
                free(pvData);
            }
            RegCloseKey(hKey);
        }
        return lang;
    }
}

namespace NS_File
{
    bool GetFilesList(const wstring &path, list<wstring> *lst, wstring &error, bool ignore_locked, bool folders_only)
    {
        wstring searchPath = toNativeSeparators(path) + L"\\*";
        if (searchPath.size() > MAX_PATH - 1) {
            error = wstring(L"Path name is too long: ") + searchPath;
            return false;
        }

        WIN32_FIND_DATA ffd;
        HANDLE hFind = FindFirstFile(searchPath.c_str(), &ffd);
        if (hFind == INVALID_HANDLE_VALUE) {
            if (ignore_locked && dirExists(path))
                return true;
            error = wstring(L"FindFirstFile invalid handle value: ") + searchPath;
            return false;
        }

        do {
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!wcscmp(ffd.cFileName, L".") || !wcscmp(ffd.cFileName, L".."))
                    continue;
                if (folders_only) {
                    lst->push_back(L"/" + wstring(ffd.cFileName));
                    continue;
                }
                if (!GetFilesList(path + L"/" + wstring(ffd.cFileName), lst, error, ignore_locked)) {
                    FindClose(hFind);
                    return false;
                }
            } else {
                if (!folders_only)
                    lst->push_back(path + L"/" + wstring(ffd.cFileName));
            }

        } while (FindNextFile(hFind, &ffd) != 0);

        if (GetLastError() != ERROR_NO_MORE_FILES) {
            FindClose(hFind);
            error = wstring(L"Error while find files: ") + searchPath;
            return false;
        }
        FindClose(hFind);
        return true;
    }

    std::vector<wstring> findFilesByPattern(const wstring &path, const wstring &pattern)
    {
        std::vector<wstring> result;
        wstring searchPath = toNativeSeparators(path) + L"\\" + pattern;
        if (searchPath.size() > MAX_PATH - 1) {
            return result;
        }

        WIN32_FIND_DATAW ffd;
        HANDLE hFind = FindFirstFile(searchPath.c_str(), &ffd);
        if (hFind == INVALID_HANDLE_VALUE)
            return result;

        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                result.push_back(L"/" + wstring(ffd.cFileName));
            }

        } while (FindNextFile(hFind, &ffd) != 0);

        FindClose(hFind);
        return result;
    }

    bool readFile(const wstring &filePath, list<wstring> &linesList)
    {
        std::wifstream file(filePath.c_str(), std::ios_base::in);
        if (!file.is_open()) {
            NS_Logger::WriteLog(L"An error occurred while opening: " + filePath);
            return false;
        }
        wstring line;
        while (std::getline(file, line))
            linesList.push_back(line);

        file.close();
        return true;
    }

    bool readBinFile(const wstring &filePath, list<wstring> &linesList)
    {
        std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            NS_Logger::WriteLog(L"An error occurred while opening: " + filePath);
            return false;
        }
        while (file.peek() != EOF) {
            WORD len = 0;
            file.read((char*)(&len), sizeof(WORD));
            if (file.fail()) {
                NS_Logger::WriteLog(L"An error occurred while reading: " + filePath);
                file.close();
                return false;
            }
            wstring line;
            line.resize(len);
            file.read((char*)&line[0], len * sizeof(wchar_t));
            if (file.fail()) {
                NS_Logger::WriteLog(L"An error occurred while reading: " + filePath);
                file.close();
                return false;
            }
            linesList.push_back(std::move(line));
        }
        file.close();
        return true;
    }

    bool writeToFile(const wstring &filePath, list<wstring> &linesList)
    {
        std::wofstream file(filePath.c_str(), std::ios_base::out);
        if (!file.is_open()) {
            NS_Logger::WriteLog(L"An error occurred while writing: " + filePath);
            return false;
        }
        for (auto &line : linesList)
            file << line << std::endl;

        file.close();
        return true;
    }

    bool writeToBinFile(const wstring &filePath, list<wstring> &linesList)
    {
        std::ofstream file(filePath.c_str(), std::ios::binary | std::ios::app);
        if (!file.is_open()) {
            NS_Logger::WriteLog(L"An error occurred while writing: " + filePath);
            return false;
        }
        for (auto &line : linesList) {
            WORD len = line.length();
            file.write((const char*)&len, sizeof(WORD));
            if (file.fail()) {
                NS_Logger::WriteLog(L"An error occurred while writing: " + filePath);
                file.close();
                return false;
            }
            file.write((const char*)line.c_str(), len * sizeof(wchar_t));
            if (file.fail()) {
                NS_Logger::WriteLog(L"An error occurred while writing: " + filePath);
                file.close();
                return false;
            }
        }
        file.close();
        return true;
    }

    bool runProcess(const wstring &fileName, const wstring &args)
    {
        wstring _args(L"\"" + fileName + L"\"");
        if (!args.empty())
            _args += L" " + args;
        if (NS_Utils::isRunAsApp()) {
            STARTUPINFO si;
            ZeroMemory(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(pi));
            if (CreateProcess(NULL, &_args[0],
                                 NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT,
                                 NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                return true;
            }
            return false;
        }

        HANDLE hTokenDup = NULL;
        if (!GetDuplicateToken(hTokenDup)) {
            return false;
        }

        LPVOID lpvEnv = NULL;
        if (!CreateEnvironmentBlock(&lpvEnv, hTokenDup, TRUE)) {
            CloseHandle(hTokenDup);
            return false;
        }

        STARTUPINFO si;
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.lpDesktop = const_cast<LPWSTR>(L"Winsta0\\Default");
        PROCESS_INFORMATION pi;
        if (CreateProcessAsUser(hTokenDup, NULL,
                                &_args[0],
                                NULL, NULL, FALSE,
                                CREATE_UNICODE_ENVIRONMENT,
                                lpvEnv, NULL, &si, &pi))
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            DestroyEnvironmentBlock(lpvEnv);
            CloseHandle(hTokenDup);
            return true;
        }
        DestroyEnvironmentBlock(lpvEnv);
        CloseHandle(hTokenDup);
        return false;
    }

    bool isProcessRunning(const wstring &filePath)
    {
        wstring fileName = filePath.substr(filePath.find_last_of(L"\\/") + 1);
        HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapShot == INVALID_HANDLE_VALUE)
            return false;

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);
        if (!Process32First(snapShot, &entry)) {
            CloseHandle(snapShot);
            return false;
        }

        do {
            if (lstrcmpi(entry.szExeFile, fileName.c_str()) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);
                if (hProcess) {
                    WCHAR processPath[MAX_PATH];
                    DWORD pathSize = MAX_PATH;
                    if (QueryFullProcessImageNameW(hProcess, 0, processPath, &pathSize)) {
                        if (lstrcmpi(processPath, filePath.c_str()) == 0) {
                            CloseHandle(hProcess);
                            CloseHandle(snapShot);
                            return true;
                        }
                    }
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(snapShot, &entry));

        CloseHandle(snapShot);
        return false;
    }

    bool fileExists(const wstring &filePath)
    {
        DWORD attr = ::GetFileAttributes(filePath.c_str());
        return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
    }

    bool dirExists(const wstring &dirName) {
        DWORD attr = ::GetFileAttributes(dirName.c_str());
        return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
    }

    bool dirIsEmpty(const wstring &dirName)
    {
        return PathIsDirectoryEmpty(dirName.c_str());
    }

    bool makePath(const wstring &path, size_t root_offset) {
        size_t len = path.length();
        if (len == 0)
            return false;
        if (CreateDirectoryW(path.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS)
            return true;
        if (len >= MAX_PATH || root_offset >= len)
            return false;
        wchar_t buf[MAX_PATH];
        wcscpy(buf, path.c_str());
        if (buf[len - 1] == '/' || buf[len - 1] == '\\')
            buf[len - 1] = '\0';
        wchar_t *it = buf + root_offset;
        while (1) {
            while (*it != '\0' && *it != '/' && *it != '\\')
                it++;
            wchar_t tmp = *it;
            *it = '\0';
            if (CreateDirectoryW(buf, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
                *it = tmp;
                return false;
            }
            if (tmp == '\0')
                break;
            *it++ = tmp;
        }
        return true;
    }

    bool replaceFile(const wstring &oldFilePath, const wstring &newFilePath)
    {
        return MoveFileExW(oldFilePath.c_str(), newFilePath.c_str(), MOVEFILE_REPLACE_EXISTING |
                              MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED) != 0;
    }

    bool replaceFolder(const wstring &from, const wstring &to, bool remove_existing)
    {
        if (!dirExists(from) || !dirExists(parentPath(to))) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            return false;
        }

        if (remove_existing && dirExists(to) && !removeDirRecursively(to)) {
            NS_Logger::WriteLog(L"Can't remove dir: " + to);
            return false;
        }

        wstring pFrom = toNativeSeparators(from);
        HRESULT hr = PerformFileOperation(pFrom, [&to](IFileOperation *pfo, IShellItem *pSrcItem) -> HRESULT {
            IShellItem *pDstItem;
            wstring pTo = toNativeSeparators(to);
            wstring pParentTo = parentPath(pTo);
            HRESULT hr = SHCreateItemFromParsingName(pParentTo.c_str(), NULL, IID_PPV_ARGS(&pDstItem));
            if (SUCCEEDED(hr)) {
                LPWSTR folderName = PathFindFileName(pTo.c_str());
                hr = pfo->MoveItem(pSrcItem, pDstItem, folderName, NULL);
                if (SUCCEEDED(hr)) {
                    hr = pfo->PerformOperations();
                }
                pDstItem->Release();
            }
            return hr;
        });
        if (FAILED(hr))
            NS_Logger::WriteLog(L"Can't move file from " + from + L" to " + to + L". HRESULT: " + std::to_wstring(hr));
        return SUCCEEDED(hr);
    }

    bool removeFile(const wstring &filePath, bool safeMode)
    {
        if (!safeMode)
            return DeleteFileW(filePath.c_str()) != 0;

        HANDLE hFile = CreateFileW(filePath.c_str(), DELETE | SYNCHRONIZE, 
                                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                   NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            if (error == ERROR_FILE_NOT_FOUND) {
                return true;
            }
            if (error == ERROR_SHARING_VIOLATION || error == ERROR_LOCK_VIOLATION) {
                // File is locked, skipping deletion
                return false;
            }
            return false;
        }

        // Get attributes via handle to avoid TOCTOU
        FILE_ATTRIBUTE_TAG_INFO tagInfo;
        if (!GetFileInformationByHandleEx(hFile, FileAttributeTagInfo, &tagInfo, sizeof(tagInfo))) {
            CloseHandle(hFile);
            return false;
        }

        if (tagInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            CloseHandle(hFile);
            return false;
        }

        if (tagInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            CloseHandle(hFile);
            // Refusing to delete reparse point
            return false;
        }        

        FILE_DISPOSITION_INFO fdi = { TRUE };
        BOOL result = SetFileInformationByHandle(hFile, FileDispositionInfo, &fdi, sizeof(fdi));
        CloseHandle(hFile);

        return result != FALSE;
    }

    bool removeDirRecursively(const wstring &dir)
    {
        if (!dirExists(dir)) {
            NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE);
            return false;
        }

        wstring pFrom = toNativeSeparators(dir);
        HRESULT hr = PerformFileOperation(pFrom, [](IFileOperation *pfo, IShellItem *pSrcItem) -> HRESULT {
            HRESULT hr = pfo->DeleteItem(pSrcItem, NULL);
            if (SUCCEEDED(hr)) {
                hr = pfo->PerformOperations();
            }
            return hr;
        });
        if (FAILED(hr))
            NS_Logger::WriteLog(L"Can't remove file from " + dir + L". HRESULT: " + std::to_wstring(hr));
        return SUCCEEDED(hr);
    }

    wstring fromNativeSeparators(const wstring &path)
    {
        wstring _path(path);
        std::replace(_path.begin(), _path.end(), L'\\', L'/');
        return _path;
    }

    wstring toNativeSeparators(const wstring &path)
    {
        wstring _path(path);
        std::replace(_path.begin(), _path.end(), L'/', L'\\');
        return _path;
    }

    wstring parentPath(const wstring &path)
    {
        size_t len = path.length();
        if (len > 1) {
            const wchar_t *buf = path.c_str();
            const wchar_t *it = buf + len - 1;
            while (*it == '/' || *it == '\\') {
                if (it == buf)
                    return L"";
                it--;
            }
            while (*it != '/' && *it != '\\') {
                if (it == buf)
                    return L"";
                it--;
            }
            if (it == buf)
                return L"";
            return wstring(buf, it - buf);
        }
        return L"";
    }

    wstring fallbackTempPath()
    {
        wstring path(L"C:/ProgramData"), dest_path = path + TEXT("/" VER_PRODUCTNAME_STR) + L" Temp";
        if (!dirExists(dest_path) && CreateDirectory(dest_path.c_str(), NULL) == 0) {
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            return path;
        }
        return dest_path;
    }

    wstring tempPath()
    {
        if (NS_Utils::isRunAsApp()) {
            WCHAR buff[MAX_PATH + 2] = {0};
            DWORD res = ::GetTempPath(MAX_PATH + 1, buff);
            if (res != 0) {
                buff[res - 1] = '\0';
                return fromNativeSeparators(buff);
            }
            NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
            return fallbackTempPath();
        }

        HANDLE hTokenDup = NULL;
        if (!GetDuplicateToken(hTokenDup)) {
            return fallbackTempPath();
        }

        WCHAR buff[MAX_PATH] = {0};
        if (ExpandEnvironmentStringsForUser(hTokenDup, L"%TEMP%", buff, MAX_PATH)) {
            CloseHandle(hTokenDup);
            return fromNativeSeparators(buff);
        }
        CloseHandle(hTokenDup);
        NS_Logger::WriteLog(ADVANCED_ERROR_MESSAGE);
        return fallbackTempPath();
    }

    wstring appPath()
    {
        WCHAR buff[MAX_PATH];
        DWORD res = ::GetModuleFileName(NULL, buff, MAX_PATH);
        return (res != 0) ? fromNativeSeparators(parentPath(buff)) : L"";
    }

    wstring getFileHash(const wstring &fileName)
    {
        HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return L"";

        // Get handle to the crypto provider
        HCRYPTPROV hProv = 0;
        if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            CloseHandle(hFile);
            return L"";
        }

        HCRYPTHASH hHash = 0;
        if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
            CloseHandle(hFile);
            CryptReleaseContext(hProv, 0);
            return L"";
        }

        DWORD cbRead = 0;
        BYTE rgbFile[BUFSIZE];
        BOOL bResult = FALSE;
        while ((bResult = ReadFile(hFile, rgbFile, BUFSIZE, &cbRead, NULL))) {
            if (cbRead == 0)
                break;

            if (!CryptHashData(hHash, rgbFile, cbRead, 0)) {
                CryptReleaseContext(hProv, 0);
                CryptDestroyHash(hHash);
                CloseHandle(hFile);
                return L"";
            }
        }

        if (!bResult) {
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return L"";
        }

        DWORD cbHashSize = 0, dwCount = sizeof(DWORD);
        if (!CryptGetHashParam( hHash, HP_HASHSIZE, (BYTE*)&cbHashSize, &dwCount, 0)) {
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return L"";
        }

        std::vector<BYTE> buffer(cbHashSize);
        if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(&buffer[0]), &cbHashSize, 0)) {
            CryptReleaseContext(hProv, 0);
            CryptDestroyHash(hHash);
            CloseHandle(hFile);
            return L"";
        }

        std::wostringstream oss;
        for (std::vector<BYTE>::const_iterator it = buffer.begin(); it != buffer.end(); ++it) {
            oss.fill('0');
            oss.width(2);
            oss << std::hex << static_cast<const int>(*it);
        }

        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CloseHandle(hFile);
        return oss.str();
    }

    bool verifyEmbeddedSignature(const wstring &fileName)
    {
        WINTRUST_FILE_INFO wfi;
        ZeroMemory(&wfi, sizeof(wfi));
        wfi.cbStruct = sizeof(WINTRUST_FILE_INFO);
        wfi.pcwszFilePath = fileName.c_str();
        wfi.hFile = NULL;
        wfi.pgKnownSubject = NULL;

        WINTRUST_DATA wtd;
        ZeroMemory(&wtd, sizeof(wtd));
        wtd.cbStruct = sizeof(WINTRUST_DATA);
        wtd.pPolicyCallbackData = NULL;
        wtd.pSIPClientData = NULL;
        wtd.dwUIChoice = WTD_UI_NONE;
        wtd.fdwRevocationChecks = WTD_REVOKE_NONE;
        wtd.dwUnionChoice = WTD_CHOICE_FILE;
        wtd.dwStateAction = WTD_STATEACTION_VERIFY;
        wtd.hWVTStateData = NULL;
        wtd.pwszURLReference = NULL;
        wtd.dwUIContext = 0;
        wtd.pFile = &wfi;

        GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2;
        LONG res = WinVerifyTrust(NULL, &action, &wtd);

        if (wtd.hWVTStateData) {
            wtd.dwStateAction = WTD_STATEACTION_CLOSE;
            WinVerifyTrust(NULL, &action, &wtd);
        }

        return (res == ERROR_SUCCESS);
    }
}

namespace NS_Logger
{
    bool allow_write_log = false;

    void AllowWriteLog()
    {
        allow_write_log = true;
    }

    void WriteLog(const wstring &log, bool showMessage)
    {
        if (allow_write_log) {
            wstring filpPath(NS_File::tempPath() + L"/oo_service_log.txt");
            std::wofstream file(filpPath.c_str(), std::ios::app);
            if (!file.is_open()) {
                return;
            }
            file << log << wstring(L"\n") << std::endl;
            file.close();
        }
        if (showMessage)
            NS_Utils::ShowMessage(log);
    }
}

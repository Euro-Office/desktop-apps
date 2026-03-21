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

#include "cdownloader.h"
#include <Windows.h>
#include <Winhttp.h>
#include <future>

#define DNL_OK         0
#define DNL_ABORT      1
#define DNL_URL_ERR    2
#define DNL_CONN_ERR   3
#define DNL_OUT_MEM    4
#define DNL_CREAT_ERR  5
#define DNL_OTHER_ERR  6


struct Connection {
    ~Connection() {
        if (hRequest) WinHttpCloseHandle(hRequest);
        if (hConnect) WinHttpCloseHandle(hConnect);
        if (hSession) WinHttpCloseHandle(hSession);
    }
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
};

static int initConnection(const wstring &url, DWORD &dwFileSize, Connection &conn)
{
    URL_COMPONENTS urlComp;
    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwHostNameLength = 1;
    urlComp.dwUrlPathLength = 1;
    if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp))
        return DNL_URL_ERR;

    wstring url_host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    wstring url_path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

    conn.hSession = WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!conn.hSession)
        return DNL_OTHER_ERR;

    DWORD dwEnabledProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
    WinHttpSetOption(conn.hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwEnabledProtocols, sizeof(DWORD));

    conn.hConnect = WinHttpConnect(conn.hSession, url_host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!conn.hConnect)
        return DNL_CONN_ERR;

    conn.hRequest = WinHttpOpenRequest(conn.hConnect, L"GET", url_path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!conn.hRequest)
        return DNL_CONN_ERR;

    if (!WinHttpSendRequest(conn.hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
        return DNL_OTHER_ERR;

    if (!WinHttpReceiveResponse(conn.hRequest, NULL))
        return DNL_CONN_ERR;

    DWORD dwStatusCode = 0, dwSize = sizeof(DWORD);
    if (!WinHttpQueryHeaders(conn.hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX))
        return DNL_CONN_ERR;

    if (dwStatusCode >= HTTP_STATUS_BAD_REQUEST)
        return DNL_CONN_ERR;

    if (!WinHttpQueryHeaders(conn.hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwFileSize, &dwSize, WINHTTP_NO_HEADER_INDEX))
        dwFileSize = 0;

    return DNL_OK;
}

class CDownloaderPrivate
{
public:
    CDownloaderPrivate()
    {}
    ~CDownloaderPrivate()
    {}

    int downloadToFile()
    {
        DWORD dwSize = 0, dwProgress = 0, dwProgressMax = 0;
        Connection conn;
        int result = initConnection(m_url, dwProgressMax, conn);
        if (result != DNL_OK)
            return result;

        HANDLE hFile = CreateFile(m_filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return DNL_CREAT_ERR;

        int prev_percent = -1;
        do {
            if (!m_run) {
                result = DNL_ABORT;
                break;
            }

            dwSize = 0;
            if (!WinHttpQueryDataAvailable(conn.hRequest, &dwSize)) {
                result = DNL_CONN_ERR;
                break;
            }

            LPSTR lpBuffer = new char[dwSize];
            if (!lpBuffer) {
                result = DNL_OUT_MEM;
                break;
            }

            DWORD dwDownloaded = 0;
            if (!WinHttpReadData(conn.hRequest, (LPVOID)lpBuffer, dwSize, &dwDownloaded)) {
                result = DNL_CONN_ERR;
                delete[] lpBuffer;
                break;
            }

            DWORD dwBytesWritten = 0;
            BOOL write_res = WriteFile(hFile, lpBuffer, dwDownloaded, &dwBytesWritten, NULL);
            delete[] lpBuffer;
            if (!write_res || dwBytesWritten != dwDownloaded) {
                result = DNL_OUT_MEM;
                break;
            }

            if (dwProgressMax != 0 && m_progress_callback) {
                dwProgress += dwDownloaded;
                int percent = static_cast<int>((100.0 * dwProgress) / dwProgressMax);
                if (percent != prev_percent) {
                    m_progress_callback(percent);
                    prev_percent = percent;
                }
            }

        } while (dwSize > 0);

        CloseHandle(hFile);
        if (result == DNL_ABORT)
            DeleteFile(m_filePath.c_str());
        return result;
    }
    FnVoidIntInt m_query_callback = nullptr;
    FnVoidInt m_complete_callback = nullptr,
              m_progress_callback = nullptr;
    wstring   m_url,
              m_filePath;
    std::future<void> m_future;
    std::atomic_bool  m_run,
                      m_lock;
};

CDownloader::CDownloader() :
    pimpl(new CDownloaderPrivate)
{
    pimpl->m_run = true;
    pimpl->m_lock = false;
}

CDownloader::~CDownloader()
{
    pimpl->m_run = false;
    if (pimpl->m_future.valid())
        pimpl->m_future.wait();
    delete pimpl, pimpl = nullptr;
}

bool CDownloader::isUrlAccessible(const wstring &url)
{
    if (url.empty())
        return false;

    DWORD dwFileSize = 0;
    Connection conn;
    int hr = initConnection(url, dwFileSize, conn);
    return hr == DNL_OK;
}

void CDownloader::queryContentLenght(const wstring &url)
{
    if (url.empty() || pimpl->m_lock)
        return;

    pimpl->m_lock = true;
    pimpl->m_future = std::async(std::launch::async, [=]() {
        DWORD dwFileSize = 0;
        Connection conn;
        int hr = initConnection(url, dwFileSize, conn);
        int error = (hr == DNL_OK) ? 0 :
                    (hr == DNL_CONN_ERR) ? -2 :
                    (hr == DNL_URL_ERR) ? -3 : -5;

        if (pimpl->m_query_callback)
            pimpl->m_query_callback(error, dwFileSize);
        pimpl->m_lock = false;
    });
}

void CDownloader::downloadFile(const std::wstring &url, const std::wstring &filePath)
{
    pimpl->m_url.clear();
    pimpl->m_filePath.clear();
    if (url.empty() || filePath.empty() || pimpl->m_lock)
        return;

    pimpl->m_url = url;
    pimpl->m_filePath = filePath;
    start();
}

void CDownloader::start()
{
    if (pimpl->m_url.empty() || pimpl->m_filePath.empty() || pimpl->m_lock)
        return;

    pimpl->m_run = true;
    pimpl->m_lock = true;
    pimpl->m_future = std::async(std::launch::async, [=]() {
        int hr = pimpl->downloadToFile();
        int error = (hr == DNL_OK) ? 0 :
                    (hr == DNL_ABORT) ? 1 :
                    (hr == DNL_OUT_MEM) ? -1 :
                    (hr == DNL_CONN_ERR) ? -2 :
                    (hr == DNL_URL_ERR) ? -3 :
                    (hr == DNL_CREAT_ERR) ? -4 : -5;

        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(error);
        pimpl->m_lock = false;
    });
}

void CDownloader::stop()
{
    pimpl->m_run = false;
}

wstring CDownloader::GetFilePath()
{
    return pimpl->m_filePath;
}

void CDownloader::onQueryResponse(FnVoidIntInt callback)
{
    pimpl->m_query_callback = callback;
}

void CDownloader::onComplete(FnVoidInt callback)
{
    pimpl->m_complete_callback = callback;
}

void CDownloader::onProgress(FnVoidInt callback)
{
    pimpl->m_progress_callback = callback;
}

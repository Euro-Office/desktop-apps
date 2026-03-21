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
#include <curl/curl.h>
#include <future>


class CDownloaderPrivate
{
public:
    CDownloaderPrivate()
    {}
    ~CDownloaderPrivate()
    {}

    FnVoidIntInt m_query_callback = nullptr;
    FnVoidInt m_complete_callback = nullptr,
              m_progress_callback = nullptr;
    string    m_url,
              m_filePath;
    int       m_prev_percent;
    std::future<void> m_future;
    std::atomic_bool  m_run,
                      m_lock;
};

int progress_callback(void *user_data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t)
{
    CDownloaderPrivate *pdata = (CDownloaderPrivate*)user_data;
    if (dltotal > 0 && pdata->m_progress_callback) {
        int percent = static_cast<int>((100.0 * dlnow) / dltotal);
        if (percent != pdata->m_prev_percent) {
            pdata->m_progress_callback(percent);
            pdata->m_prev_percent = percent;
        }
    }

    if (pdata->m_run == false)
        return CURLE_ABORTED_BY_CALLBACK;

    return CURLE_OK;
}

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

bool CDownloader::isUrlAccessible(const string &url)
{
    if (url.empty())
        return false;

    CURLcode res = CURLE_FAILED_INIT;
    if (CURL *curl = curl_easy_init()) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return res == CURLE_OK;
}

void CDownloader::queryContentLenght(const string &url)
{
    if (url.empty() || pimpl->m_lock)
        return;

    pimpl->m_lock = true;
    pimpl->m_future = std::async(std::launch::async, [=]() {
        curl_off_t fileSize = 0;
        CURLcode res = CURLE_FAILED_INIT;
        if (CURL *curl = curl_easy_init()) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &fileSize);
                if (fileSize < 0) fileSize = 0;
            }
            curl_easy_cleanup(curl);
        }
        int error = (res == CURLE_OK) ? 0 :
                    (res == CURLE_HTTP_RETURNED_ERROR) ? -2 : -3;

        if (pimpl->m_query_callback)
            pimpl->m_query_callback(error, (int)fileSize);
        pimpl->m_lock = false;
    });
}

void CDownloader::downloadFile(const string &url, const string &filePath)
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
        CURLcode res = CURLE_FAILED_INIT;
        if (FILE *fp = fopen(pimpl->m_filePath.c_str(), "wb")) {
            if (CURL *curl = curl_easy_init()) {
                pimpl->m_prev_percent = -1;
                curl_easy_setopt(curl, CURLOPT_URL, pimpl->m_url.c_str());
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
                curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
                curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void*)pimpl);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
            }
            fclose(fp);
            if (res == CURLE_ABORTED_BY_CALLBACK)
                remove(pimpl->m_filePath.c_str());
        } else
            res = CURLE_WRITE_ERROR;

        int error = (res == CURLE_OK) ? 0 :
                    (res == CURLE_ABORTED_BY_CALLBACK) ? 1 :
                    (res == CURLE_OUT_OF_MEMORY) ? -1 :
                    (res == CURLE_HTTP_RETURNED_ERROR) ? -2 : -3;

        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(error);
        pimpl->m_lock = false;
    });
}

void CDownloader::pause()
{
    pimpl->m_run = false;
}

void CDownloader::stop()
{
    pimpl->m_run = false;
}

string CDownloader::GetFilePath()
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

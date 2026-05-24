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

#include "cunzip.h"
#include "platform_win/utils.h"
#include <Windows.h>
#include <codecvt>
#include "unzip.h"

#define MAX_PATH_LEN 512
#define BLOCK_SIZE   8192

static bool makePathA(char *path, size_t root_offset = 0) {
    char *it = path + root_offset;
    while (*it++ != '\0') {
        while (*it != '\0' && *it != '/')
            it++;
        if (*it == '\0' && *(it - 1) == '/')
            break;
        char tmp = *it;
        *it = '\0';
        if (CreateDirectoryA(path, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
            *it = tmp;
            return false;
        }
        *it = tmp;
    }
    return true;
}

class CUnzip::CUnzipPrivate
{
public:
    CUnzipPrivate()
    {}
    ~CUnzipPrivate()
    {}

    int unzipArchive(const wstring &zipFilePath, const wstring &folderPath)
    {
        if (!NS_File::fileExists(zipFilePath) || !NS_File::dirExists(folderPath))
            return UNZIP_ERROR;

        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
        std::string utf8ZipFilePath = utf8_conv.to_bytes(NS_File::fromNativeSeparators(zipFilePath));
        std::string utf8FolderPath = utf8_conv.to_bytes(NS_File::fromNativeSeparators(folderPath));

        unzFile hzf = unzOpen(utf8ZipFilePath.c_str());
        if (!hzf)
            return UNZIP_ERROR;

        unz_global_info g_info;
        if (unzGetGlobalInfo(hzf, &g_info) != UNZ_OK) {
            unzClose(hzf);
            return UNZIP_ERROR;
        }

        int prev_percent = -1;
        uLong total_count = g_info.number_entry;
        for (uLong i = 0; i < total_count; ++i) {
            if (!run) {
                unzClose(hzf);
                return UNZIP_ABORT;
            }
            unz_file_info file_info;
            char entry_name[MAX_PATH_LEN];
            if (unzGetCurrentFileInfo(hzf, &file_info, entry_name, MAX_PATH_LEN, NULL, 0, NULL, 0) != UNZ_OK) {
                unzClose(hzf);
                return UNZIP_ERROR;
            }

            char out_path[MAX_PATH_LEN];
            snprintf(out_path, MAX_PATH_LEN, "%s/%s", utf8FolderPath.c_str(), entry_name);
            if (entry_name[strlen(entry_name) - 1] == '/') {
                if (::CreateDirectoryA(out_path, NULL) == 0 && !makePathA(out_path, utf8FolderPath.size() + 1))
                    return UNZIP_ERROR;

            } else {
                if (unzOpenCurrentFile(hzf) != UNZ_OK) {
                    unzClose(hzf);
                    return UNZIP_ERROR;
                }

                FILE *hFile = fopen(out_path, "wb");
                if (!hFile) {
                    unzCloseCurrentFile(hzf);
                    unzClose(hzf);
                    return UNZIP_ERROR;
                }

                int bytes_read = 0;
                do {
                    char buff[BLOCK_SIZE] = {0};
                    bytes_read = unzReadCurrentFile(hzf, buff, BLOCK_SIZE);
                    if (bytes_read < 0 || (bytes_read > 0 && fwrite(buff, bytes_read, 1, hFile) != 1)) {
                        fclose(hFile);
                        unzCloseCurrentFile(hzf);
                        unzClose(hzf);
                        return UNZIP_ERROR;
                    }
                } while (bytes_read > 0);
                fclose(hFile);
                unzCloseCurrentFile(hzf);
            }

            if (progress_callback) {
                int percent = static_cast<int>(100.0 * (double(i + 1) / total_count));
                if (percent != prev_percent) {
                    progress_callback(percent);
                    prev_percent = percent;
                }
            }

            if ((i + 1) < total_count && unzGoToNextFile(hzf) != UNZ_OK) {
                unzClose(hzf);
                return UNZIP_ERROR;
            }
        }
        unzClose(hzf);
        return UNZIP_OK;
    }

    FnVoidInt complete_callback = nullptr,
              progress_callback = nullptr;
    std::atomic_bool run;
    std::future<void> future;
};

CUnzip::CUnzip() :
    pimpl(new CUnzipPrivate)
{
    pimpl->run = false;
}

CUnzip::~CUnzip()
{
    pimpl->run = false;
    if (pimpl->future.valid())
        pimpl->future.wait();
    delete pimpl, pimpl = nullptr;
}

void CUnzip::extractArchive(const wstring &zipFilePath, const wstring &folderPath)
{
    pimpl->run = false;
    if (pimpl->future.valid())
        pimpl->future.wait();
    pimpl->run = true;
    pimpl->future = std::async(std::launch::async, [=]() {
        int res = pimpl->unzipArchive(zipFilePath, folderPath);
        if (pimpl->complete_callback)
            pimpl->complete_callback(res);
    });
}

void CUnzip::stop()
{
    pimpl->run = false;
}

void CUnzip::onComplete(FnVoidInt callback)
{
    pimpl->complete_callback = callback;
}

void CUnzip::onProgress(FnVoidInt callback)
{
    pimpl->progress_callback = callback;
}

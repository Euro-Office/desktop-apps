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
#include "platform_linux/utils.h"
#include <archive.h>
#include <archive_entry.h>
#include <sys/stat.h>
#include <cstring>

#define BLOCK_SIZE 10240


class CUnzip::CUnzipPrivate
{
public:
    CUnzipPrivate()
    {}
    ~CUnzipPrivate()
    {}

    int unzipArchive(const string &zipFilePath, const string &folderPath, string &error)
    {
        if (!NS_File::fileExists(zipFilePath) || !NS_File::dirExists(folderPath)) {
            error = "Archive path is empty or dest dir not exist";
            return UNZIP_ERROR;
        }
        int prev_percent = -1;
        struct stat file_stat;
        long total_size = (stat(zipFilePath.c_str(), &file_stat) == 0) ? file_stat.st_size : 0;

        struct archive *arch = archive_read_new();
        archive_read_support_filter_xz(arch);
        archive_read_support_format_tar(arch);
        if (archive_read_open_filename(arch, zipFilePath.c_str(), BLOCK_SIZE) != ARCHIVE_OK) {
            error = "Cannot open archive";
            archive_read_free(arch);
            return UNZIP_ERROR;
        }

        int res = ARCHIVE_OK;
        int ex_code = UNZIP_OK;
        struct archive_entry *entry;
        while ((res = archive_read_next_header(arch, &entry)) == ARCHIVE_OK) {
            if (!run) {
                ex_code = UNZIP_ABORT;
                break;
            }

            const char *entryname = archive_entry_pathname(entry);
            if (!entryname) {
                error = "Invalid entry name";
                break;
            }

            char outpath[1024] = {0};
            snprintf(outpath, sizeof(outpath), "%s/%s", folderPath.c_str(), entryname);

            if (archive_entry_filetype(entry) == AE_IFREG) {
                archive_entry_set_pathname(entry, outpath);
                res = archive_read_extract(arch, entry, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM
                                                            | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS
                                                            | ARCHIVE_EXTRACT_NO_OVERWRITE);
                if (res != ARCHIVE_OK) {
                    error = "Cannot extract entry";
                    break;
                }
                if (total_size > 0 && progress_callback) {
                    int percent = static_cast<int>(100.0 * ((double)archive_filter_bytes(arch, -1)/total_size));
                    if (percent != prev_percent) {
                        progress_callback(percent);
                        prev_percent = percent;
                    }
                }
            }
        }

        if (res != ARCHIVE_EOF && ex_code != UNZIP_ABORT) {
            error = string("Error reading archive: ") + archive_error_string(arch);
            ex_code = UNZIP_ERROR;
        }

        archive_read_close(arch);
        archive_read_free(arch);
        return ex_code;
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

void CUnzip::extractArchive(const string &zipFilePath, const string &folderPath)
{
    pimpl->run = false;
    if (pimpl->future.valid())
        pimpl->future.wait();
    pimpl->run = true;
    pimpl->future = std::async(std::launch::async, [=]() {
        string error;
        int res = pimpl->unzipArchive(zipFilePath, folderPath, error);
        if (!error.empty())
            fprintf(stderr, "%s", error.c_str());
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

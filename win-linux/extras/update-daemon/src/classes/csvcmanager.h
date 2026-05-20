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

#ifndef CSVCMANAGER_H
#define CSVCMANAGER_H

#include "classes/csocket.h"
#include <future>
#ifdef _WIN32
# include "classes/platform_win/cdownloader.h"
# include "classes/platform_win/cunzip.h"
#else
# include "classes/platform_linux/cdownloader.h"
# include "classes/platform_linux/cunzip.h"
#endif

typedef std::function<void(void)> FnVoidVoid;
using std::future;


class CSvcManager
{
public:
    explicit CSvcManager();
    ~CSvcManager();

    /* callback */
    void aboutToQuit(FnVoidVoid callback);

private:
    void init();
    void onQueryResponse(const int error, const int lenght);
    void onCompleteUnzip(const int error);
    void onCompleteSlot(const int error, const tstring &filePath);
    void onProgressSlot(const int percent);
    void unzipIfNeeded(const tstring &filePath, const tstring &newVersion);
    void clearTempFiles(const tstring &prefix, const tstring &except = tstring());
    void startReplacingFiles(const tstring &packageType, const bool restartAfterUpdate);
    void startReplacingService(const bool restartAfterUpdate);
#ifdef _WIN32
    void startInstallPackage();
#endif

    struct PackageData;
    struct SavedPackageData;
    PackageData      *m_packageData;
    SavedPackageData *m_savedPackageData;

    FnVoidVoid   m_quit_callback = nullptr;
    tstring      m_checkUrl,
                 m_currVersion,
                 m_ignVersion,
                 m_newVersion;
    bool         m_lock = false;
    int          m_downloadMode,
                 m_packageType;
    future<void> m_future_clear;
    CSocket     *m_socket = nullptr;
    CDownloader *m_pDownloader = nullptr;
    CUnzip      *m_pUnzip = nullptr;

    enum Mode {
        CHECK_UPDATES=0, DOWNLOAD_CHANGELOG=1, DOWNLOAD_UPDATES=2
    };
    enum Package {
        ISS = 0, MSI, Portable, Other
    };
};

#endif // CSVCMANAGER_H

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

#ifndef CSOCKET_H
#define CSOCKET_H

#include <string>
#include <vector>
#include <functional>
#ifdef _WIN32
# include <tchar.h>
# define tchar wchar_t
# define tstringstream std::wstringstream
# define tstring std::wstring
# define to_tstring to_wstring
#else
# define _T(str) str
# define tchar char
# define tstringstream std::stringstream
# define tstring std::string
# define to_tstring to_string
#endif

using std::size_t;

typedef std::function<void(void*, size_t)> FnVoidData;
typedef std::function<void(const char*)> FnVoidCharPtr;


enum MsgCommands {
    MSG_CheckUpdates = 0,
    MSG_LoadUpdates,
    MSG_LoadCheckFinished,
    MSG_LoadUpdateFinished,
    MSG_UnzipIfNeeded,
    MSG_ShowStartInstallMessage,
    MSG_StartReplacingFiles,
    MSG_ClearTempFiles,
    MSG_Progress,
    MSG_StopDownload,
    MSG_OtherError,
    MSG_RequestContentLenght,
    MSG_UnzipProgress,
    MSG_SetLanguage,
    MSG_StartReplacingService,
    MSG_StartInstallPackage
};

class CSocket
{
public:
    CSocket(int sender_port, int receiver_port, bool retry_connect = true, bool use_unique_addr = false);
    ~CSocket();

    /* callback */
    bool isPrimaryInstance();
    bool sendMessage(void *data, size_t size);
    bool sendMessage(int cmd, const tstring &param1 = _T(""), const tstring &param2 = _T(""));
    void onMessageReceived(FnVoidData callback);
    void onError(FnVoidCharPtr callback);
    int  parseMessage(void *data, std::vector<tstring> &params);

private:
    class CSocketPrv;
    CSocketPrv *pimpl = nullptr;
};

#endif // CSOCKET_H

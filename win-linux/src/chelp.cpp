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

#include "chelp.h"
#include <stdio.h>
#ifdef _WIN32
# include <Windows.h>
#endif


CHelp::CHelp()
{

}

void CHelp::out()
{
    const char help[] =
        "\n\n"
        "Desktop editors for cloud portal.\n"
        "\n"
        "keys:\n"
        "    --custom-title-bar turns off system title bar\n"
        "    --system-title-bar turns on system title bar\n"
        "    --keeplang=en keeps the language\n"
        "    --lang=en applies the language for the current session\n"
        "    --new=[doc|cell|slide] creates new document/spreadsheet/presentation\n"
        "    --review=path/to/file to open document in review mode in separate window\n"
        "    --view=path/to/file to open document in view mode in separate window\n"
        "    --force-use-tab document will be opend in new tab instead of new separate window\n"
        "    --geometry=default reset window geometry\n"
        "    --xdg-desktop-portal use portals instead of gtk file chooser (the flag is saved for subsequent sessions)\n"
        "    --xdg-desktop-portal=default use portals instead of gtk file chooser for current session\n"
        "    --native-file-dialog use non Qt dialog\n"
        "    --lock-portals force hide the Connect to the cloud button on the start page"
        "    --unlock-portals disable forced hiding of the Connect to the cloud button on the start page"
        "    --updates-appcast-channel=dev set development URL for updates\n"
        "    --updates-interval=<time> set update check interval in seconds (minimum 30 sec)\n"
        "    --updates-reset reset all update options\n";

#ifdef _WIN32
    FILE *pFile = NULL;
    if (AttachConsole(ATTACH_PARENT_PROCESS))
        freopen_s(&pFile, "CONOUT$", "w", stdout);
#endif
    fprintf(stdout, help);
    fflush(stdout);
#ifdef _WIN32
    if (pFile)
        fclose(pFile);
#endif
}

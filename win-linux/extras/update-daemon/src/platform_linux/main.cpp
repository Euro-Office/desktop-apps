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

#include "platform_linux/utils.h"
#include "classes/platform_linux/capplication.h"
#include "classes/platform_linux/ctimer.h"
#include "classes/csvcmanager.h"
#include "classes/translator.h"
#include "version.h"
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"
#include <csignal>
#include <cstring>
#include <locale>

#define DECL_VERSION __attribute__((section(".version_info"), unused))

volatile static const char DECL_VERSION version[] = VER_FILEVERSION_STR;
static const char gSvcVersion[] = "Service version: " VER_FILEVERSION_STR;

void strToNum(const char *str, int &num)
{
    char *err = NULL;
    int _num = strtol(str, &err, 10);
    if (!err || *err == '\0')
        num = _num;
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        if (strcmp(argv[1], "--run-as-app") == 0) {
            NS_Utils::parseCmdArgs(argc, argv);
            if (NS_Utils::cmdArgContains("--log")) {
                NS_Logger::AllowWriteLog();
                NS_Logger::WriteLog(gSvcVersion);
            }
            std::locale::global(std::locale(""));
            Translator::instance().init(NS_Utils::GetAppLanguage(), "/langs/langs.bin");
            CSocket socket(0, INSTANCE_SVC_PORT);
            if (!socket.isPrimaryInstance())
                return 0;

            int pid = -1;
            if (argc > 2)
                strToNum(argv[2], pid);

            CApplication app;
            CSvcManager upd;
            socket.onMessageReceived([&app, &pid](void *buff, size_t) {
                if (strcmp((const char*)buff, "stop") == 0)
                    app.exit(0);
                else
                    strToNum((const char*)buff, pid);
            });

            // Termination on crash of the main application
            CTimer tmr;
            tmr.start(30000, [&app, &pid]() {
                if (pid != -1 && kill(pid, 0) != 0)
                    app.exit(0);
            });
            return app.exec();
        }
    }

    return 0;
}

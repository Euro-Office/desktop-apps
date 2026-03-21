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

#include <QApplication>
#include <QProcess>
#include <QFileInfo>
#include <Windows.h>
#ifndef __OS_WIN_XP
# include "jumplist.h"
# include "shellassoc.h"
# include <shlobj_core.h>
#endif


void SetAppUserModelId()
{
    if (HMODULE lib = LoadLibrary(L"shell32")) {
        HRESULT (WINAPI *SetAppUserModelID)(PCWSTR AppID);
        *(FARPROC*)&SetAppUserModelID = GetProcAddress(lib, "SetCurrentProcessExplicitAppUserModelID");
        if (SetAppUserModelID)
            SetAppUserModelID(TEXT(APP_USER_MODEL_ID));
        FreeLibrary(lib);
    }
}


int main(int argc, char *argv[])
{
    SetAppUserModelId();

    QApplication a(argc, argv);
//    return a.exec();

    QStringList _cmdArgs(QCoreApplication::arguments().mid(1));
#ifndef __OS_WIN_XP
    if (_cmdArgs.contains("--create-jump-list")) {
        CreateJumpList();
        return 0;
    } else
    if (_cmdArgs.contains("--remove-jump-list")) {
        ClearHistory();
        DeleteJumpList();
        return 0;
    } else
    if (_cmdArgs.contains("--assoc")) {
        if (_cmdArgs.size() > 1) {
            std::wstring assocLine = _cmdArgs.at(1).toStdWString();
            size_t len = assocLine.length();
            if (len == 0)
                return 0;

            std::vector<AssocPair> assocList;
            wchar_t *buf = &assocLine[0];
            if (buf[len - 1] == ';')
                buf[len - 1] = '\0';

            size_t last_sep_pos = 0;
            wchar_t *it = buf;
            while (1) {
                while (*it != '\0' && *it != ';')
                    it++;
                wchar_t tmp = *it;
                *it = '\0';
                wchar_t *pair = buf + last_sep_pos;
                if (wchar_t *colon = wcschr(pair, L':')) {
                    *colon = L'\0';
                    assocList.push_back({pair, colon + 1});
                }
                if (tmp == '\0')
                    break;
                last_sep_pos = it - buf + 1;
                it++;
            }
            if (!assocList.empty())
                SetUserFileAssoc(assocList);
        }
        return 0;
    }
#endif

    QFileInfo _fi(QString::fromLocal8Bit(argv[0]));

    qputenv("Path", "./converter;" + qgetenv("Path"));

#define APP_LAUNCH_NAME "./DesktopEditors.exe"

//    QProcess::startDetached(fi.absolutePath() + APP_LAUNCH_NAME, _cmdArgs, fi.absolutePath());
    QProcess::startDetached(_fi.absolutePath() + "./editors.exe", _cmdArgs, _fi.absolutePath());

    return 0;
}

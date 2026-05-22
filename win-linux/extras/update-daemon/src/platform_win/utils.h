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

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <list>
#include <vector>

using std::string;
using std::wstring;
using std::to_wstring;
using std::list;

#define FUNCTION_INFO wstring(TEXT(__FUNCTION__)) + L" Line: " + to_wstring(__LINE__)
#define DEFAULT_ERROR_MESSAGE _TR(MESSAGE_TEXT_ERR1) + L" " + FUNCTION_INFO
#define ADVANCED_ERROR_MESSAGE DEFAULT_ERROR_MESSAGE + \
    L" " + NS_Utils::GetLastErrorAsString()

namespace NS_Utils
{
void setRunAsApp();
bool isRunAsApp();
void parseCmdArgs(int argc, wchar_t *argv[]);
bool cmdArgContains(const wstring &param);
wstring cmdArgValue(const wstring &param);
wstring cmdArgsAsString();
wstring GetLastErrorAsString();
int ShowMessage(wstring str, bool showError = false);
wstring GetAppLanguage();
}

namespace NS_File
{
bool GetFilesList(const wstring &path, list<wstring> *lst, wstring &error, bool ignore_locked = false, bool folders_only = false);
std::vector<wstring> findFilesByPattern(const wstring &path, const wstring &pattern);
bool readFile(const wstring &filePath, list<wstring> &linesList);
bool readBinFile(const wstring &filePath, list<wstring> &linesList);
bool writeToFile(const wstring &filePath, list<wstring> &linesList);
bool writeToBinFile(const wstring &filePath, list<wstring> &linesList);
bool runProcess(const wstring &fileName, const wstring &args);
bool isProcessRunning(const wstring &filePath);
bool fileExists(const wstring &filePath);
bool dirExists(const wstring &dirName);
bool dirIsEmpty(const wstring &dirName);
bool makePath(const wstring &path, size_t root_offset = 3);
bool replaceFile(const wstring &oldFilePath, const wstring &newFilePath);
bool replaceFolder(const wstring &from, const wstring &to, bool remove_existing = false);
bool removeFile(const wstring &filePath, bool safeMode = false);
bool removeDirRecursively(const wstring &dir);
wstring fromNativeSeparators(const wstring &path);
wstring toNativeSeparators(const wstring &path);
wstring parentPath(const wstring &path);
wstring tempPath();
wstring appPath();
wstring getFileHash(const wstring &fileName);
bool verifyEmbeddedSignature(const wstring &fileName);
}

namespace NS_Logger
{
void AllowWriteLog();
void WriteLog(const wstring &log, bool showMessage = false);
}

#endif // UTILS_H

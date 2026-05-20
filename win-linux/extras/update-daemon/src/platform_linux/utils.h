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
#include "classes/translator.h"

using std::string;
using std::to_string;
using std::list;

#define FUNCTION_INFO string(__FUNCTION__) + " Line: " + to_string(__LINE__)
#define DEFAULT_ERROR_MESSAGE _TR(MESSAGE_TEXT_ERR1) + " " + FUNCTION_INFO
#define ADVANCED_ERROR_MESSAGE DEFAULT_ERROR_MESSAGE + \
    " " + NS_Utils::GetLastErrorAsString()

namespace NS_Utils
{
void parseCmdArgs(int argc, char *argv[]);
bool cmdArgContains(const string &param);
string cmdArgValue(const string &param);
string GetLastErrorAsString();
int ShowMessage(string str, bool showError = false);
//string GetSysLanguage();
string GetAppLanguage();
}

namespace NS_File
{
bool GetFilesList(const string &path, list<string> *lst, string &error, bool ignore_locked = false, bool folders_only = false);
bool readFile(const string &filePath, list<string> &linesList);
bool writeToFile(const string &filePath, list<string> &linesList);
bool runProcess(const string &fileName, const string &args);
bool isProcessRunning(const string &fileName);
bool fileExists(const string &filePath);
bool dirExists(const string &dirName);
bool dirIsEmpty(const string &dirName);
bool makePath(const string &path, size_t root_offset = 1);
bool replaceFile(const string &oldFilePath, const string &newFilePath);
bool replaceFolder(const string &from, const string &to, bool remove_existing = false);
bool removeFile(const string &filePath);
bool removeDirRecursively(const string &dir);
string parentPath(const string &path);
string tempPath();
string appPath();
string getFileHash(const string &fileName);
//bool verifyEmbeddedSignature(const string &fileName);
}

namespace NS_Logger
{
void AllowWriteLog();
void WriteLog(const string &log, bool showMessage = false);
}

#endif // UTILS_H

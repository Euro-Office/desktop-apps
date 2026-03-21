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

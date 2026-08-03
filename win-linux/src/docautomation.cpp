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
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "docautomation.h"
#include "defines.h"
#include "utils.h"
#include "../../../desktop-sdk/ChromiumBasedEditors/lib/include/applicationmanager.h"
#include "../../../desktop-sdk/ChromiumBasedEditors/lib/src/x2t.h"
#include "../../../core/DesktopEditor/common/File.h"
#include "../../../core/DesktopEditor/common/Directory.h"
#include "../../../core/DesktopEditor/common/StringBuilder.h"

#include <iostream>

namespace {
    std::wstring GetFlagValue(const std::wstring& flag) {
        return InputArgs::contains(flag) ? InputArgs::argument_value(flag) : L"";
    }

    // First non-flag argument is treated as the input file path.
    std::wstring GetInputFileArg() {
        for (const auto& arg : InputArgs::arguments()) {
            if (arg.rfind(L"--", 0) != 0) {
                return arg;
            }
        }
        return L"";
    }
}

bool NSDocAutomation::ShouldRun() {
    return InputArgs::contains(L"--convert-to") || InputArgs::contains(L"--print-to-file");
}

int NSDocAutomation::Run() {
    bool bPrintToFile = InputArgs::contains(L"--print-to-file");
    std::wstring sOutput = bPrintToFile ? GetFlagValue(L"--print-to-file") : GetFlagValue(L"--convert-to");
    std::wstring sInput = GetInputFileArg();

    if (sInput.empty() || sOutput.empty()) {
        std::wcerr << L"Usage: DesktopEditors <input-file> --convert-to=<output-file>" << std::endl;
        std::wcerr << L"       DesktopEditors <input-file> --print-to-file=<output-file.pdf>" << std::endl;
        return 1;
    }

    if (!NSFile::CFileBinary::Exists(sInput)) {
        std::wcerr << L"Input file not found: " << sInput << std::endl;
        return 1;
    }

    // --print-to-file is an alias for --convert-to that forces PDF output,
    // since x2t's PDF export is already what "Print" ultimately produces.
    if (bPrintToFile) {
        std::wstring sPdfExt = L".pdf";
        if (sOutput.length() < sPdfExt.length() ||
            0 != sOutput.compare(sOutput.length() - sPdfExt.length(), sPdfExt.length(), sPdfExt)) {
            sOutput += sPdfExt;
        }
    }

    // A plain CAscApplicationManager, not the QObject-based GUI singleton
    // (AscAppManager::getInstance()) -- that one installs Qt event filters
    // in its constructor and crashes without a live QCoreApplication, which
    // doesn't exist yet at this point (before any CEF/Qt startup).
    CAscApplicationManager oManager;
    QString sUserDataPath = Utils::getUserPath() + APP_DATA_PATH;
    oManager.m_oSettings.SetUserDataPath(sUserDataPath.toStdWString());
    std::wstring sAppPath = NSFile::GetProcessDirectory();
    oManager.m_oSettings.file_converter_path = sAppPath + L"/converter";

    std::wstring sTempDir = NSDirectory::CreateDirectoryWithUniqueName(NSFile::CFileBinary::GetTempPath());

    // Formats are left for x2t to detect from the file extensions, the same
    // way its own `x2t <from> <to>` mode does.
    NSStringUtils::CStringBuilder oBuilder;
    oBuilder.WriteString(L"<?xml version=\"1.0\" encoding=\"utf-8\"?><TaskQueueDataConvert><m_sFileFrom>");
    oBuilder.WriteEncodeXmlString(sInput);
    oBuilder.WriteString(L"</m_sFileFrom><m_sFileTo>");
    oBuilder.WriteEncodeXmlString(sOutput);
    oBuilder.WriteString(L"</m_sFileTo><m_sFontDir>");
    oBuilder.WriteEncodeXmlString(oManager.m_oSettings.fonts_cache_info_path);
    oBuilder.WriteString(L"</m_sFontDir><m_sAllFontsPath>");
    oBuilder.WriteEncodeXmlString(oManager.m_oSettings.fonts_cache_info_path + L"/AllFonts.js");
    oBuilder.WriteString(L"</m_sAllFontsPath><m_sTempDir>");
    oBuilder.WriteEncodeXmlString(sTempDir);
    oBuilder.WriteString(L"</m_sTempDir></TaskQueueDataConvert>");

    // The ".xml" suffix is load-bearing: x2t only parses its argument as a
    // TaskQueueDataConvert file when the name ends in ".xml" (see
    // X2tConverter/src/main.cpp). Anything else is taken as the first of a
    // bare `x2t <from> <to>` pair, leaving the destination empty and failing
    // with "Empty sFileFrom or sFileTo".
    std::wstring sTempName = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"CLI");
    if (NSFile::CFileBinary::Exists(sTempName))
        NSFile::CFileBinary::Remove(sTempName);
    std::wstring sXmlPath = sTempName + L".xml";
    NSFile::CFileBinary::SaveToFile(sXmlPath, oBuilder.GetData(), true);

    int nReturnCode = NSX2T::Convert(oManager.m_oSettings.file_converter_path + L"/x2t", sXmlPath, &oManager, true);

    NSFile::CFileBinary::Remove(sXmlPath);
    NSDirectory::DeleteDirectory(sTempDir);

    if (0 == nReturnCode) {
        std::wcout << L"OK: " << sOutput << std::endl;
    } else {
        std::wcerr << L"Conversion failed (x2t exit code " << nReturnCode << L")" << std::endl;
    }

    return nReturnCode;
}

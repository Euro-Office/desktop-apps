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

#ifndef CEDITORTOOLS_H
#define CEDITORTOOLS_H

#include "qascprinter.h"
#include "cascapplicationmanagerwrapper.h"
#include "components/cprintdialog.h"

namespace CEditorTools
{
    struct sPrintConf
    {
        sPrintConf(CCefView * v, QAscPrinterContext * c, QVector<PageRanges> *ranges, ParentHandle p)
            : view(v)
            , context(c)
            , page_ranges(ranges)
            , parent(p)
        {}

        CCefView * view;
        QAscPrinterContext * context;
        QVector<PageRanges> *page_ranges;
        ParentHandle parent;
    };

    void print(const sPrintConf&);
    void onDocumentPrint(QWidget *parent, CCefView *pView, const QString &documentName, int currentPage, int pagesCount);
    void getlocalfile(void * data);
    QString getlocalfile(const std::wstring& path, int parentid = -1);
    QString getlocaltemplate(const std::wstring& editor, int parentid);
    QString getlocaltheme(int parentid);
    std::wstring getFolder(const std::wstring&, int parentid = -1);

    auto createEditorPanel(const COpenOptions& opts, QWidget *parent = nullptr) -> CTabPanel *;
    auto editorTypeFromFormat(int format) -> AscEditorType;
    auto processLocalFileSaveAs(const NSEditorApi::CAscCefMenuEvent * event) -> void;
}

#endif // CEDITORTOOLS_H

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

#ifndef ASCEDITORWIDGET
#define ASCEDITORWIDGET

#include <QWidget>
#include <QTabWidget>
#include <QTabBar>
#include <QStyleOption>
#include <QPainter>
#include <QApplication>

namespace NSAscEditor
{
    enum EditorType
    {
        etDocument      = 0,
        etPresentation  = 1,
        etSpreadsheet   = 2
    };
}

#if 0
class CAscEditorWidget : public QWidget
{
    Q_OBJECT

public:
    NSAscEditor::EditorType m_etType;

public:
    CAscEditorWidget(QWidget *parent = 0, NSAscEditor::EditorType etType = NSAscEditor::etDocument) : QWidget(parent)
    {
        m_etType = etType;
        setStyleSheet("background-color:#FF00FF");
    }

    ~CAscEditorWidget()
    {
    }

    void SetIcon(int nIndex, QTabWidget* pWidget)
    {
        int nSelectedIndex = pWidget->tabBar()->currentIndex();

        QTabBar* pTabBar = pWidget->tabBar();

        bool bIsActive = (nIndex == nSelectedIndex) ? true : false;
        switch (m_etType)
        {
            case NSAscEditor::etPresentation:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/Icons/tabicon_PE_active.png") : QIcon(":/Icons/tabicon_PE_normal.png"));
                break;
            }
            case NSAscEditor::etSpreadsheet:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/Icons/tabicon_SE_active.png") : QIcon(":/Icons/tabicon_SE_normal.png"));
                break;
            }
            case NSAscEditor::etDocument:
            default:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/Icons/tabicon_DE_active.png") : QIcon(":/Icons/tabicon_DE_normal.png"));
                break;
            }
        }

        pTabBar->setTabTextColor(nIndex, bIsActive ? QColor(255, 255, 255) : QColor(51, 51, 51));
    }

protected:
    void paintEvent(QPaintEvent* e)
    {
        Q_UNUSED(e);

        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
};
#else
#include "./cef/qcefwebview.h"
class CAscEditorWidget : public QCefWebView
{
    Q_OBJECT

public:
    NSAscEditor::EditorType m_etType;

public:
    CAscEditorWidget(QWidget *parent = 0, NSAscEditor::EditorType etType = NSAscEditor::etDocument, std::wstring strUrl = L"") : QCefWebView(parent)
    {
        this->setParentTabs((QTabWidget*)parent);
        m_etType = etType;

        std::wstring sUrl = QApplication::applicationDirPath().toStdWString();
        sUrl += L"/deploy/apps/api/documents/index.html";

        if (etType == NSAscEditor::etSpreadsheet)
            sUrl += L"?doctype=spreadsheet";
        if (etType == NSAscEditor::etPresentation)
            sUrl += L"?doctype=presentation";

        if (strUrl.length() != 0)
            sUrl = strUrl;

        this->load(sUrl);
    }

    virtual ~CAscEditorWidget()
    {
    }

    void SetIcon(int nIndex, QTabWidget* pWidget)
    {
        int nSelectedIndex = pWidget->tabBar()->currentIndex();

        QTabBar* pTabBar = pWidget->tabBar();

        bool bIsActive = (nIndex == nSelectedIndex) ? true : false;

        switch (m_etType)
        {
            case NSAscEditor::etPresentation:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/res/icons/tabicon_PE_active.png") : QIcon(":/res/icons/tabicon_PE_normal.png"));
                break;
            }
            case NSAscEditor::etSpreadsheet:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/res/icons/tabicon_SE_active.png") : QIcon(":/res/icons/tabicon_SE_normal.png"));
                break;
            }
            case NSAscEditor::etDocument:
            default:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/res/icons/tabicon_DE_active.png") : QIcon(":/res/icons/tabicon_DE_normal.png"));
                break;
            }
        }

        //pTabBar->setTabTextColor(nIndex, bIsActive ? QColor(255, 255, 255) : QColor(51, 51, 51));
        pTabBar->setTabTextColor(nIndex, QColor(51, 51, 51));
    }
};
#endif

#endif // ASCEDITORWIDGET


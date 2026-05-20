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

#ifndef CTABPANEL_H
#define CTABPANEL_H

#include <QWidget>
#include <QTimer>

#include "qcefview.h"
#include "casctabdata.h"

class CTabPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CTabPanel(QWidget *parent, const QSize& s);
    explicit CTabPanel(QWidget *parent, const QSize& s, const QColor& c);
    ~CTabPanel();

    static CTabPanel * createEditorPanel(QWidget *parent, const QSize& size);

    QCefView * view() const;
    CCefView * cef() const;
    void setView(QCefView *);

    CAscTabData * data() const;
    void setData(CAscTabData *);

    void initAsEditor();
    void initAsSimple();

    void openLocalFile(const std::wstring& sFilePath, int nFileFormat, const std::wstring& params);
    bool openLocalFile(const std::wstring& sFilePath, const std::wstring& params);
    void createLocalFile(AscEditorType nFileFormat, const std::wstring& sName = L"");
    void createLocalFile(const std::wstring& templatepath, const std::wstring& name);
    void createLocalFile(int templateid, const std::wstring& name);
    bool openRecoverFile(int id);
    bool openRecentFile(int id);
    bool isReady();
    bool hasUncommittedChanges();
    void applyLoader(const QString& cmd, const QString& args = QString());
    void setReady();
    //void resize(int w, int h);
    //void resizeEvent(QResizeEvent *event);
    //void showEvent(QShowEvent *event);

    bool prettyTitle() { return m_prettyTitle; }
    void setPrettyTitle(bool v) { m_prettyTitle = v; }
    void setBackground(const QColor&);

    bool reporterMode() const;
    void setReporterMode(bool);

protected:
    //void timerEvent(QTimerEvent *event);
    //void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QCefView *      m_pViewer;
    QCefView *      m_pLoader = nullptr;
    CAscTabData *   m_pData = nullptr;

    QSize m_startSize, m_lastSize;
    int m_idTimerResize = 0;
    bool m_prettyTitle = false;
    bool m_isReady = false;
    bool m_isReporterOn = false;

signals:
    void closePanel(QCloseEvent *event);

public slots:
    void showFullScreen();
    void showNormal();
};

#endif // CTABPANEL_H

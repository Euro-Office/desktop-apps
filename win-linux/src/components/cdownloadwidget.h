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

#ifndef CDOWNLOADWIDGET_H
#define CDOWNLOADWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include "cpushbutton.h"


class CDownloadWidget : public QWidget
{
    Q_OBJECT
    struct CDownloadItem;
    typedef std::map<int, CDownloadItem *>::const_iterator MapItem;

public:
    explicit CDownloadWidget(QWidget *parent = 0);
    ~CDownloadWidget();

    void downloadProcess(void *);
    QPushButton * toolButton();
    void updateScalingFactor(double);
    void applyTheme();
    void onLayoutDirectionChanged();

protected:
    virtual void closeEvent(QCloseEvent *) final;
    virtual bool eventFilter(QObject*, QEvent*) final;

private:
    QWidget * addFile(const QString&, int);
    QString getFileName(const QString&) const;
    void removeFile(MapItem);
    void onStart();
    void onFinish(int);
    void clearlAll();
    void polish();

    CPushButton * m_pToolButton = nullptr;
    QScrollArea * m_pArea = nullptr;
    QWidget *m_pContentArea = nullptr;
    QFrame *m_mainFrame = nullptr,
           *m_titleFrame = nullptr;
    std::map<int, CDownloadItem *> m_mapDownloads;
    double m_dpiRatio = 1;
};

#endif // CDOWNLOADWIDGET_H

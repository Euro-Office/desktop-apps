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

#ifndef CCEFEVENTSGATE_H
#define CCEFEVENTSGATE_H

#include <QObject>
#include "components/ctabpanel.h"

class CCefEventsGate : public QObject
{
    Q_OBJECT

public:
    explicit CCefEventsGate(QObject *parent = nullptr);

    virtual void init(CTabPanel * const);
    CTabPanel * const panel()
    {
        return m_panel;
    }

protected:
    CTabPanel * m_panel = nullptr;

public slots:
    virtual void onPortalLogout(std::wstring portal) = 0;
    virtual void onEditorConfig(int id, std::wstring cfg) = 0;
    virtual void onEditorActionRequest(int, const QString&) = 0;
    virtual void onDocumentName(void *);
    virtual void onDocumentChanged(int id, bool changed);
    virtual void onDocumentSave(int id, bool cancel = false);
    virtual void onDocumentSaveInnerRequest(int id) = 0;
    virtual void onDocumentFragmented(int id, bool needbuild) = 0;
    virtual void onDocumentFragmentedBuild(int id, int error);
    virtual void onDocumentPrint(void *);
    virtual void onDocumentPrint(int current, uint count) = 0;
    virtual void onDocumentLoadFinished(int);
    virtual void onDocumentReady(int);
    virtual void onDocumentType(int id, int type);

    virtual void onFileLocation(int id, QString path) = 0;
    virtual void onLocalFileSaveAs(void *);

    virtual void onEditorAllowedClose(int) = 0;
    virtual void onKeyDown(void *);
    virtual void onFullScreen(int id, bool apply) = 0;

    virtual void onWebTitleChanged(int, std::wstring json) = 0;
    virtual void onWebAppsFeatures(int, std::wstring) = 0;
    virtual void onReporterMode(int, bool);
    virtual void onImageLoadFinished(int err) = 0;
    virtual void onErrorPage(int id, const std::wstring& action);
};

#endif // CCEFEVENTSGATE_H

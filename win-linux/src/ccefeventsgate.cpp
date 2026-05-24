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


#include "ccefeventsgate.h"
#include "utils.h"
#include "common/Types.h"

using namespace NSEditorApi;

CCefEventsGate::CCefEventsGate(QObject *parent)
    : QObject(parent)
{
}

void CCefEventsGate::init(CTabPanel * const p)
{
    m_panel = p;
}

void CCefEventsGate::onDocumentChanged(int, bool changed)
{
    CAscTabData * doc = m_panel->data();

    /* TODO: if exists the saving error, sdk rise the changing event
     * again. maybe not a good action.
    */
    if (doc->closed() && changed) doc->reuse();
    /**/

    if (doc->hasChanges() != changed && (!doc->closed() || changed)) {
        doc->setChanged(changed);
    }
}

void CCefEventsGate::onDocumentName(void * data)
{
    CAscDocumentName * pData = static_cast<CAscDocumentName *>(data);

    CAscTabData * doc = m_panel->data();
    doc->setTitle(QString::fromStdWString(pData->get_Name()));
    if ( doc->isLocal() ) {
        if ( pData->get_Url().empty() && !pData->get_Path().empty() ) {
            doc->setUrl(Utils::replaceBackslash(QString::fromStdWString(pData->get_Path())));
        }
    }

    RELEASEINTERFACE(pData);
}

void CCefEventsGate::onDocumentType(int id, int type)
{
    if ( m_panel->cef()->GetId() == id )
        m_panel->data()->setContentType(AscEditorType(type));
}

void CCefEventsGate::onDocumentSave(int, bool cancel)
{
    CAscTabData * doc = m_panel->data();
    if ( doc->closed() && cancel ) {
        doc->reuse();
    }
}

void CCefEventsGate::onDocumentPrint(void * data)
{
    CAscPrintEnd * pData = reinterpret_cast<CAscPrintEnd *>(data);

    onDocumentPrint(pData->get_CurrentPage(), pData->get_PagesCount());

    RELEASEINTERFACE(pData);
}

void CCefEventsGate::onDocumentFragmentedBuild(int, int error)
{
    if ( error && m_panel->data()->closed() ) {
        m_panel->data()->reuse();
    }
}

void CCefEventsGate::onKeyDown(void *)
{

}

void CCefEventsGate::onDocumentLoadFinished(int uid)
{
}

void CCefEventsGate::onDocumentReady(int)
{
}

void CCefEventsGate::onLocalFileSaveAs(void *)
{
}

void CCefEventsGate::onReporterMode(int, bool m)
{
    panel()->setReporterMode(m);
}

void CCefEventsGate::onErrorPage(int, const std::wstring&)
{}

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

#include "ccefeventstransformer.h"
#include "applicationmanager_events.h"
#include "common/Types.h"
#include <QProcess>
#include <QTimer>

using namespace NSEditorApi;

CCefEventsTransformer::CCefEventsTransformer(QObject *parent)
    : CAscCefMenuEventListener()
    , pObjTarget(parent)
{
    qRegisterMetaType<std::wstring>("std::wstring");
}

void CCefEventsTransformer::OnEvent(NSEditorApi::CAscCefMenuEvent *pEvent)
{
    OnEvent(pObjTarget, pEvent);
}

void CCefEventsTransformer::OnEvent(QObject * target, NSEditorApi::CAscCefMenuEvent * event)
{
    if (NULL == event || nullptr == target)
        return;

    switch (event->m_nType) {
//    case ASC_MENU_EVENT_TYPE_CEF_CREATETAB: {
//        CAscCreateTab * pData = (CAscCreateTab *)event->m_pData;

//        break;}

    case ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE: {
        CAscTabEditorType * pData = (CAscTabEditorType *)event->m_pData;

        QMetaObject::invokeMethod(target, "onDocumentType", Qt::QueuedConnection, Q_ARG(int, pData->get_Id()), Q_ARG(int, pData->get_Type()));
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME: {
        CAscDocumentName * pData = (CAscDocumentName *)event->m_pData;

        ADDREFINTERFACE(pData)
        QMetaObject::invokeMethod(target, "onDocumentName", Qt::QueuedConnection, Q_ARG(void*, pData));
        break; }

    case ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED: {
        NSEditorApi::CAscDocumentModifyChanged * pData = (NSEditorApi::CAscDocumentModifyChanged *)event->m_pData;

        QMetaObject::invokeMethod(target, "onDocumentChanged", Qt::QueuedConnection,
                                    Q_ARG(int, pData->get_Id()), Q_ARG(bool, pData->get_Changed()));

        break;}

    case ASC_MENU_EVENT_TYPE_CEF_ONSAVE: {
        CAscDocumentOnSaveData * pData = (CAscDocumentOnSaveData *)event->m_pData;
        QMetaObject::invokeMethod(target, "onDocumentSave", Qt::QueuedConnection,
                                  Q_ARG(int, pData->get_Id()), Q_ARG(bool, pData->get_IsCancel()));
        break;}

    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO: {
        CAscEditorSaveQuestion * pData = (CAscEditorSaveQuestion *)event->m_pData;
        QMetaObject::invokeMethod(target, "onDocumentSaveInnerRequest", Qt::QueuedConnection, Q_ARG(int, pData->get_Id()));
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT: {
        /* will be used synchronous action after logout immediately */
//        QMetaObject::invokeMethod(target, "onLogout", Qt::QueuedConnection);
        break;}

//    case ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE: { // deprecated
//        NSEditorApi::CAscJSMessage * pData = (NSEditorApi::CAscJSMessage *)event->m_pData;
//        QString cmd = QString::fromStdWString(pData->get_Name());
//        if (cmd.compare("login") == 0) {
//            QMetaObject::invokeMethod(pObjParent, "onLogin", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdWString(pData->get_Value())));
//        }
//        break; }

    case ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END:
    case ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END_ERROR: {
        int _error = event->m_nType == ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END_ERROR ? -1 : 0;
        QMetaObject::invokeMethod(target, "onDocumentFragmentedBuild", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()), Q_ARG(int, _error));
        break; }

    case ASC_MENU_EVENT_TYPE_CEF_ONCLOSE: break;
    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE: break;
    case ASC_MENU_EVENT_TYPE_CEF_DESTROYWINDOW:
        QMetaObject::invokeMethod(target, "onEditorAllowedClose", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()));
        break;

    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS: break;

    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END: {
#ifdef Q_OS_LINUX
        QTimer::singleShot(0, pObjTarget, [target]{
            QMetaObject::invokeMethod(target, "onDocumentPrint", Qt::QueuedConnection, Q_ARG(void*, nullptr));
        });
#else
        QMetaObject::invokeMethod(target, "onDocumentPrint", Qt::QueuedConnection, Q_ARG(void*, nullptr));
#endif
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK: break;

    case ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN: {
        NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)event->m_pData;

        ADDREFINTERFACE(pData)
        QMetaObject::invokeMethod(target, "onKeyDown", Qt::QueuedConnection, Q_ARG(void*, pData));
        break; }

    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER:
    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE:
        QMetaObject::invokeMethod(target, "onFullScreen", Qt::QueuedConnection,
                        Q_ARG(int, event->get_SenderId()), Q_ARG(bool, event->m_nType == ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER));
        break;

    case ASC_MENU_EVENT_TYPE_REPORTER_CREATE:
    case ASC_MENU_EVENT_TYPE_REPORTER_END: {
        QMetaObject::invokeMethod(target, "onReporterMode", Qt::QueuedConnection,
                        Q_ARG(int, event->get_SenderId()), Q_ARG(bool, event->m_nType == ASC_MENU_EVENT_TYPE_REPORTER_CREATE));
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN: {
        CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)event->m_pData;
        ADDREFINTERFACE(pData);

        QMetaObject::invokeMethod(target, "onLocalFilesOpen", Qt::QueuedConnection, Q_ARG(void*, pData));
        break; }

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN:
    /*case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN:*/ {
        CAscLocalOpenFileRecent_Recover * pData = (CAscLocalOpenFileRecent_Recover *)event->m_pData;

        ADDREFINTERFACE(pData);
        QMetaObject::invokeMethod(target, "onLocalFileRecent", Qt::QueuedConnection, Q_ARG(void*, pData));
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE: {
        CAscLocalSaveFileDialog * pData = (CAscLocalSaveFileDialog *)event->m_pData;

        ADDREFINTERFACE(pData);
        QMetaObject::invokeMethod(target, "onLocalFileSaveAs", Qt::QueuedConnection, Q_ARG(void*, pData));
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_PORTAL_OPEN: {
        CAscExecCommand * pData = (CAscExecCommand *)event->m_pData;
        QMetaObject::invokeMethod( target, "onPortalOpen", Qt::QueuedConnection,
                            Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
        break; }

    case ASC_MENU_EVENT_TYPE_PAGE_LOAD_END:
        QMetaObject::invokeMethod( target, "onDocumentLoadFinished", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()) );
        break;

    case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND: {
        CAscExecCommand * const pData = (CAscExecCommand *)event->m_pData;
        const std::wstring& cmd = pData->get_Command();

        if (cmd.compare(L"portal:open") == 0) {
            QMetaObject::invokeMethod( target, "onPortalOpen", Qt::QueuedConnection,
                    Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
        } else
        if ( !(cmd.find(L"portal:new") == std::wstring::npos) ) {
            QMetaObject::invokeMethod( target, "onPortalNew", Qt::QueuedConnection,
                    Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
        } else
        if ( !(cmd.find(L"portal:create") == std::wstring::npos) ) {
            QMetaObject::invokeMethod( target, "onPortalCreate", Qt::QueuedConnection);
        } else
        if ( !(cmd.find(L"auth:sso") == std::wstring::npos) ||
                !(cmd.find(L"auth:outer") == std::wstring::npos) ) {
            QMetaObject::invokeMethod( target, "onOutsideAuth", Qt::QueuedConnection,
                    Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
        } else
        if (cmd.compare(L"portal:logout") == 0) {
            QMetaObject::invokeMethod( target, "onPortalLogout", Qt::QueuedConnection, Q_ARG(std::wstring, pData->get_Param()) );
        } else
//        } else
//        if ( !(cmd.find(L"files:check") == std::wstring::npos) ) {
//            QMetaObject::invokeMethod( target, "onLocalFilesCheck", Qt::QueuedConnection,
//                    Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
        if ( cmd.compare(L"portal:uitheme") == 0 ) {
            QMetaObject::invokeMethod(target, "onPortalUITheme", Qt::QueuedConnection,
                                      Q_ARG(int, event->get_SenderId()), Q_ARG(std::wstring, pData->get_Param()));
        } else
        if ( !(cmd.find(L"files:explore") == std::wstring::npos) ) {
                QMetaObject::invokeMethod( target, "onLocalFileLocation", Qt::QueuedConnection,
                    Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
        } else
        if ( !(cmd.find(L"go:folder") == std::wstring::npos) ) {
            QMetaObject::invokeMethod( target, "onFileLocation", Qt::QueuedConnection,
                            Q_ARG(int, event->get_SenderId()), Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
        } else
        if ( !(cmd.find(L"doc:onready") == std::wstring::npos) ) {
            QMetaObject::invokeMethod( target, "onDocumentReady", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()) );
        } else
        if ( !(cmd.find(L"editor:event") == std::wstring::npos) ||
                !(cmd.find(L"editor:request") == std::wstring::npos) ) {
            QMetaObject::invokeMethod( target, "onEditorActionRequest", Qt::QueuedConnection,
                                       Q_ARG(int, event->get_SenderId()), Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
        } else
        /* webapps:events is deprecated. used for back compatibility for awhile */
        if ( !(cmd.find(L"webapps:events") == std::wstring::npos) ) {
            QMetaObject::invokeMethod( target, "onWebAppsFeatures", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()), Q_ARG(std::wstring, L"{\"eventloading\":true}") );
        } else
        if ( !(cmd.find(L"webapps:features") == std::wstring::npos) ) {
            QMetaObject::invokeMethod(target, "onWebAppsFeatures", Qt::QueuedConnection,
                            Q_ARG(int, event->get_SenderId()), Q_ARG(std::wstring, pData->get_Param()));
        } else
        if ( !(cmd.find(L"editor:config") == std::wstring::npos) ) {
            QMetaObject::invokeMethod( target, "onEditorConfig", Qt::QueuedConnection,
                            Q_ARG(int, event->get_SenderId()), Q_ARG(std::wstring, pData->get_Param()) );
        } else
        if ( cmd.compare(L"encrypt:isneedbuild") == 0 ) {
            bool isFragmented = pData->get_Param() == L"true" ? true : false;
            QMetaObject::invokeMethod(target, "onDocumentFragmented", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()), Q_ARG(bool, isFragmented));
        } else
        if ( cmd.compare(L"error:page") == 0 ) {
            QMetaObject::invokeMethod(target, "onErrorPage", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()), Q_ARG(std::wstring, pData->get_Param()));
        } else {
//            std::wregex _re_appcmd(L"^app\\:(\\w+)", std::tr1::wregex::icase);
//            auto _iter_cmd = std::wsregex_iterator(cmd.begin(), cmd.end(), _re_appcmd);
//            if (_iter_cmd != std::wsregex_iterator()) {
//                std::wsmatch _cmd_match = *_iter_cmd;

//                cmd = _cmd_match.str(1);
                if (cmd.find(L"app:onready") != std::wstring::npos) {
                    QMetaObject::invokeMethod( target, "onDocumentReady", Qt::QueuedConnection, Q_ARG(int, -1) );
                } else
                if (cmd.find(L"app:localoptions") != std::wstring::npos) {
                    QMetaObject::invokeMethod( target, "onLocalOptions", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdWString(pData->get_Param())) );
                }
//            }
        }

        break; }
    }

    RELEASEINTERFACE(event);
}

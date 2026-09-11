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

//
//  ASCEventsController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/23/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCEventsController.h"
#import "ASCConstants.h"
#import "ASCExternalController.h"
#import "ASCPresentationReporter.h"
#import "ASCSharedSettings.h"
#import "NSDictionary+Extensions.h"
#import "NSString+Extensions.h"
#import "OfficeFileFormats.h"
#import "ASCLinguist.h"
#import "mac_application.h"
#import "mac_cefviewmedia.h"
#import "NSApplication+Extensions.h"
#import "ASCEditorJSVariables.h"
#import "ASCThemesController.h"

#pragma mark -
#pragma mark ========================================================
#pragma mark ASCEventListener
#pragma mark ========================================================
#pragma mark -

class ASCEventListener: public NSEditorApi::CAscCefMenuEventListener {
    dispatch_queue_t eventListenerQueue;
    id <ASCExternalDelegate> externalDelegate;
    int fullscreen_view_id = -1;
public:
    ASCEventListener() : NSEditorApi::CAscCefMenuEventListener () {
        eventListenerQueue = dispatch_queue_create("asc.onlyoffice.MenuEventListenerQueue", NULL);
        externalDelegate = [[ASCExternalController shared] delegate];
    }
    
    virtual void OnEvent(NSEditorApi::CAscCefMenuEvent* pRawEvent)
    {
        if (NULL == pRawEvent)
            return;
     
//        if (pRawEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START) {
//            NSLog(@"ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD_START");
//        }
        
        __block NSEditorApi::CAscMenuEvent * pEvent = pRawEvent;
        
        dispatch_async(eventListenerQueue, ^{
            dispatch_async(dispatch_get_main_queue(), ^{
                int senderId = -1;

                NSEditorApi::CAscCefMenuEvent * pCefEvent = dynamic_cast<NSEditorApi::CAscCefMenuEvent *>(pEvent);
                if (pCefEvent) {
                    senderId = pCefEvent->get_SenderId();
                }

                switch (pEvent->m_nType) {
                    case ASC_MENU_EVENT_TYPE_CEF_CREATETAB: {
                        NSEditorApi::CAscCreateTab *pData = (NSEditorApi::CAscCreateTab*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"action"  : @(ASCTabActionOpenUrl),
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_IdEqual()],
                                                                                     @"url"     : [NSString stringWithstdwstring:pData->get_Url()],
                                                                                     @"active"  : @(pData->get_Active())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE: {
                        NSEditorApi::CAscTabEditorType * pData = (NSEditorApi::CAscTabEditorType*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameTabEditorType
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_Id()],
                                                                                     @"type"    : @(pData->get_Type())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONCLOSE:
                        break;
                        
                    case ASC_MENU_EVENT_TYPE_CEF_DOCUMENT_NAME: {
                        NSEditorApi::CAscDocumentName* pData = (NSEditorApi::CAscDocumentName*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameTabEditorNameChanged
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_Id()],
                                                                                     @"name"    : [NSString stringWithstdwstring:pData->get_Name()],
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()],
                                                                                     @"url"     : [NSString stringWithstdwstring:pData->get_Url()]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_MODIFY_CHANGED: {
                        NSEditorApi::CAscDocumentModifyChanged * pData = (NSEditorApi::CAscDocumentModifyChanged *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameModifyChanged
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_Id()],
                                                                                     @"сhanged" : @(pData->get_Changed())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONLOGOUT: {
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONSAVE: {
                        NSEditorApi::CAscDocumentOnSaveData* saveData = (NSEditorApi::CAscDocumentOnSaveData*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameSave
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", saveData->get_Id()],
                                                                                     @"cancel"  : @(saveData->get_IsCancel())
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_JS_MESSAGE: {
                        NSEditorApi::CAscJSMessage * pData = (NSEditorApi::CAscJSMessage *)pEvent->m_pData;
                        
                        NSRange range = [[NSString stringWithstdwstring:pData->get_Name()] rangeOfString:@"login"];
                        
                        if (range.location == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameLogin
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:pData->get_Value()] dictionary]];
                        }
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORECLOSE:
                        break;
                        
                    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD: {
                        NSEditorApi::CAscDownloadFileInfo * pData = (NSEditorApi::CAscDownloadFileInfo*)pEvent->m_pData;
                        
//                        ADDREFINTERFACE(pData);
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameDownload
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"data" : [NSValue value:&pData withObjCType:@encode(void *)]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_CHECK_KEYBOARD: {
                        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                        appManager->CheckKeyboard();
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_PROGRESS:
                        break;
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONBEFORE_PRINT_END: {
                        NSEditorApi::CAscPrintEnd * pData = (NSEditorApi::CAscPrintEnd *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePrintDialog
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId"      : @(pData->get_Id()),
                                                                                     @"countPages"  : @(pData->get_PagesCount()),
                                                                                     @"currentPage" : @(pData->get_CurrentPage()),
                                                                                     @"options"     : [NSString stringWithstdwstring: pData->get_Options()]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK: {
                        NSEditorApi::CAscOnOpenExternalLink * pData = (NSEditorApi::CAscOnOpenExternalLink *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenUrl
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"url" : [NSString stringWithstdwstring:pData->get_Url()]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN: {
                        NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameKeyboardDown
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"data" : [NSValue value:&pData withObjCType:@encode(void *)]
                                                                                     }];
                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER:
                    case ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENLEAVE: {
                        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                        CCefView * pCefView = appManager->GetViewById(pRawEvent->get_SenderId());

                        if ( pCefView && pCefView->GetType() == cvwtEditor && !((CCefViewEditor*)pCefView)->IsPresentationReporter() ) {
                            if ( pRawEvent->m_nType == ASC_MENU_EVENT_TYPE_CEF_ONFULLSCREENENTER ) {
                                if ( fullscreen_view_id < 0 ) {
                                    fullscreen_view_id = pRawEvent->get_SenderId();

                                    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                                                        object:nil
                                                                                      userInfo:@{@"viewId"       : @(pRawEvent->get_SenderId()),
                                                                                                 @"fullscreen"   : @(true)}];
                                } else {
                                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                    pCommand->put_Command(L"editor:stopDemonstration");
                                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                                    pEvent->m_pData = pCommand;
                                    pCefView->Apply(pEvent);
                                }
                            } else {
                                if ( fullscreen_view_id == pRawEvent->get_SenderId() )
                                    fullscreen_view_id = -1;

                                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                                                    object:nil
                                                                                  userInfo:@{@"viewId"       : @(pRawEvent->get_SenderId()),
                                                                                             @"fullscreen"   : @(false)}];
                            }
                        }

                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN: {
                        NSEditorApi::CAscLocalOpenFiles * pData = (NSEditorApi::CAscLocalOpenFiles *)pEvent->m_pData;
                        
                        for(std::vector<std::wstring>::iterator it = pData->get_Files().begin(); it != pData->get_Files().end(); ++it) {
                            std::wstring filePath = *it;
                            
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"action"  : @(ASCTabActionOpenLocalFile),
                                                                                         @"path"    : [NSString stringWithstdwstring:filePath],
                                                                                         @"active"  : @(YES)
                                                                                         }];
                        }
                        break;
                    }

//                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN:
//                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN: {
//                        NSEditorApi::CAscLocalOpenFileRecent_Recover* pData = (NSEditorApi::CAscLocalOpenFileRecent_Recover*)pEvent->m_pData;
//
//
//                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
//                                                                            object:nil
//                                                                          userInfo:@{
//                                                                                     @"action"  : @(ASCTabActionOpenLocalRecoverFile),
//                                                                                     @"active"  : @(YES),
//                                                                                     @"fileId"  : @(pData->get_Id()),
//                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()]
//                                                                                     }];
//                        break;
//                    }

                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE: {
                        NSEditorApi::CAscLocalSaveFileDialog* pData = (NSEditorApi::CAscLocalSaveFileDialog*)pEvent->m_pData;
                        
                        NSMutableArray * supportFormats = [NSMutableArray array];
                        
                        for(std::vector<int>::iterator it = pData->get_SupportFormats().begin(); it != pData->get_SupportFormats().end(); ++it) {
                            int type = *it;
                            
                            NSDictionary * info = [ASCConstants ascFormatsInfo][@(type)];
                            
                            if (info) {
                                [supportFormats addObject:@{
                                                            @"type"         : @(type),
                                                            @"description"  : info[@"description"],
                                                            @"extension"    : info[@"extension"]
                                                            }];
                            }
                        }
                        
                        // Begin hotfix ODP presentation
                        
                        NSArray* pptxExtension = [supportFormats filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"SELF['extension'] == %@", @"pptx"]];
                        NSArray* odpExtension  = [supportFormats filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"SELF['extension'] == %@", @"odp"]];
                        
                        if ([pptxExtension count] > 0 && [odpExtension count] < 1) {
                            NSDictionary * info = [ASCConstants ascFormatsInfo][@(AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP)];
                            [supportFormats addObject:@{
                                                        @"type"         : @(AVS_OFFICESTUDIO_FILE_PRESENTATION_ODP),
                                                        @"description"  : info[@"description"],
                                                        @"extension"    : info[@"extension"]
                                                        }];
                        }
                        
                        // End hotfix ODP presentation
                        
                        NSDictionary * formatInfo = [ASCConstants ascFormatsInfo][@(pData->get_FileType())];
                        NSDictionary * originalFileInfo = @{
                                                        @"type"         : @(pData->get_FileType()),
                                                        @"typeInfo"     : formatInfo ? formatInfo : @{}
                                                        };

                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameSaveLocal
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()],
//                                                                                     @"fileType": @(pData->get_FileType()),
                                                                                     @"original": originalFileInfo,
                                                                                     @"supportedFormats" : supportFormats,
                                                                                     @"viewId"  : [NSString stringWithFormat:@"%d", pData->get_Id()]
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE: {
                        NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenImage
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()],
                                                                                     @"fileId"  : @(pData->get_Id())
                                                                                     }];
                        break;
                    }

					case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_PLAYER_COMMAND: {
						CAscApplicationManager* appManager = [NSAscApplicationWorker getAppManager];
						CCefView* pCefView = appManager->GetViewById(pRawEvent->get_SenderId());
						if (pCefView)
						{
							CCefViewWidgetImpl* pWidgetImpl = pCefView->GetWidgetImpl();
							if (pWidgetImpl)
							{
								CCefViewMedia* pCefViewMedia = static_cast<CCefViewMedia*>(pWidgetImpl);
								pCefViewMedia->OnMediaPlayerCommand(static_cast<NSEditorApi::CAscExternalMediaPlayerCommand*>(pEvent->m_pData));
							}
						}

						break;
					}

                    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG: {
                        NSEditorApi::CAscLocalOpenFileDialog* pData = (NSEditorApi::CAscLocalOpenFileDialog*)pEvent->m_pData;

                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenFileDialog
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"path"    : [NSString stringWithstdwstring:pData->get_Path()],
                                                                                     @"filter"  : [NSString stringWithstdwstring:pData->get_Filter()],
                                                                                     @"fileId"  : @(pData->get_Id()),
                                                                                     @"isMulti" : @(pData->get_IsMultiselect())
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_REPORTER_CREATE: {                        
                        [[ASCPresentationReporter sharedInstance] create:pEvent->m_pData from:senderId];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_REPORTER_END: {
                        [[ASCPresentationReporter sharedInstance] destroy];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO:
                    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM: {
                        [[ASCPresentationReporter sharedInstance] apply:pEvent];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE: {
                        pEvent->AddRef();

                        CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];

                        if (appManager) {
                            appManager->Apply(pEvent);
                        }

                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE: {
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenSSLCertificate
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId": [NSString stringWithFormat:@"%d", senderId]
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO: {
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameSaveBeforSign
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId": [NSString stringWithFormat:@"%d", senderId]
                                                                                     }];
                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PLUGINS: {
                        NSEditorApi::CAscSystemExternalPlugins * pData = dynamic_cast<NSEditorApi::CAscSystemExternalPlugins *>(pEvent->m_pData);

                        if (pData) {
                            for (const NSEditorApi::CAscSystemExternalPlugins::CItem& item: pData->get_Items()) {
                                NSMutableDictionary * json = [[NSMutableDictionary alloc] initWithDictionary:
                                                              @{
                                                                @"name": [NSString stringWithstdwstring:item.name],
                                                                @"id": [NSString stringWithstdwstring:item.id],
                                                                @"url": [NSString stringWithstdwstring:item.url]
                                                                }];

                                if (!item.nameLocale.empty()) {
                                    NSString * currentLocale = [[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString];

                                    if (NSDictionary * nameLocales = [[NSString stringWithstdwstring:item.nameLocale] dictionary]) {
                                        if (NSString * nameLocale = nameLocales[currentLocale]) {
                                            json[@"name"] = nameLocale;
                                        }
                                    }
                                }

                                if (NSString * jsonString = [json jsonString]) {
                                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                    pCommand->put_Command(L"panel:external");
                                    pCommand->put_Param([jsonString stdwstring]);

                                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                                    pEvent->m_pData = pCommand;

                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    appManager->SetEventToAllMainWindows(pEvent);
                                }
                            }
                        }

                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END:
                    case ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END_ERROR: {
                        int error = pEvent->m_nType == ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD_END_ERROR ? -1 : 0;
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameDocumentFragmentBuild
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                     @"error": @(error)
                                                                                     }];

                        break;
                    }
                        
                    case ASC_MENU_EVENT_TYPE_WINDOW_SHOW_CERTIFICATE: {
                        NSEditorApi::CAscX509CertificateData * pData = dynamic_cast<NSEditorApi::CAscX509CertificateData *>(pEvent->m_pData);
                        
                        [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCertificatePreview
                                                                            object:nil
                                                                          userInfo:@{
                                                                                     @"text": [NSString stringWithstdwstring:pData->get_Data()],
                                                                                     @"path": [NSString stringWithstdwstring:pData->get_FilePath()]
                                                                                     }];

                        break;
                    }

                    case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND: {
                        NSEditorApi::CAscExecCommand * pData = (NSEditorApi::CAscExecCommand *)pEvent->m_pData;
                        std::wstring cmd = pData->get_Command();
                        std::wstring param = pData->get_Param();

#ifdef DEBUG
                        NSLog(@"[Start page] Command: \"%@\", params: \"%@\"", [NSString stringWithstdwstring:cmd], [NSString stringWithstdwstring:param]);
#endif

                        if (cmd.compare(L"portal:open") == 0 || cmd.find(L"auth:outer") != std::wstring::npos) {
                            NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary];
                            NSString * portal = json[@"portal"];
                            NSString * provider = json[@"provider"];

                            if (portal && provider) {
                                NSURLComponents * urlPage = [NSURLComponents componentsWithString:portal];
                                id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

                                NSMutableString * entrypage = [NSMutableString stringWithString:[json objectForKey:@"entrypage"]];
                                if ( entrypage ) {
                                    if ( [entrypage characterAtIndex:0] != '/' )
                                        [entrypage insertString:@"/" atIndex:0];

                                    if ( urlPage.path )
                                        [entrypage insertString:urlPage.path atIndex:0];

                                    NSRange pathrange = [entrypage rangeOfString:@"?"];
                                    if ( pathrange.location != NSNotFound ) {
                                        urlPage.path = [entrypage substringToIndex:pathrange.location];
                                        urlPage.query = [entrypage substringFromIndex:pathrange.location+1];
                                    } else {
                                        urlPage.path = entrypage;
                                    }
                                }

                                NSURLQueryItem *countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];

                                if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
                                    countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[externalDelegate onAppPreferredLanguage]];
                                }

                                NSURLQueryItem *portalAddress = [NSURLQueryItem queryItemWithName:@"desktop" value:@"true"];

                                NSMutableArray * qitems = urlPage.queryItems ? [NSMutableArray arrayWithArray:urlPage.queryItems] : [[NSMutableArray alloc] init];
                                [qitems addObjectsFromArray:@[countryCode, portalAddress]];
                                urlPage.queryItems = qitems;

                                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                    object:nil
                                                                                  userInfo:@{
                                                                                             @"action"  : @(ASCTabActionOpenPortal),
                                                                                             @"url"     : [urlPage string],
                                                                                             @"provider": provider,
                                                                                             @"active"  : @(YES)
                                                                                             }];
                            }
                        } else if (cmd.compare(L"portal:login") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalLogin
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                         @"info": [NSString stringWithstdwstring:param]
                                                                                         }];
                        } else if (cmd.compare(L"portal:logout") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalLogout
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:param] dictionary]];
                        } else if (cmd.compare(L"portal:create") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalCreate
                                                                                object:nil
                                                                              userInfo:nil];
                        } else if (cmd.compare(L"portal:new") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalNew
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:param] dictionary]];
                        } else if (cmd.compare(L"auth:sso") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNamePortalSSO
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:param] dictionary]];
                        } else if (cmd.compare(L"files:explore") == 0) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFileInFinder
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"info": [NSString stringWithstdwstring:param]
                                                                                         }];
                        } else if (cmd.compare(L"files:check") == 0) {                            
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFilesCheck
                                                                                object:nil
                                                                              userInfo:[[NSString stringWithstdwstring:param] dictionary]];
                        } else if (cmd.find(L"app:onready") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameStartPageReady
                                                                                object:nil
                                                                              userInfo:nil];
                        } else if (cmd.find(L"doc:onready") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorDocumentReady
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId]
                                                                                         }];
                        } else if (cmd.find(L"editor:ready") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorAppReady
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId]
                                                                                         }];
                        } else if (cmd.find(L"editor:event") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorEvent
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                         @"data": [[NSString stringWithstdwstring:param] dictionary]
                                                                                         }];
                        } else if (cmd.find(L"editor:request") != std::wstring::npos) {
                            NSMutableDictionary * params = [NSMutableDictionary dictionaryWithDictionary:@{@"viewId": [NSString stringWithFormat:@"%d", senderId]}];
                            [params addEntriesFromDictionary:[[NSString stringWithstdwstring:param] dictionary]];

                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorAppActionRequest
                                                                                object:nil
                                                                              userInfo:params];
                        } else if (cmd.find(L"recent:forget") != std::wstring::npos) {
                            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                            appManager->RemoveRecentByViewId(senderId);
                        } else if (cmd.find(L"recent:pinned") != std::wstring::npos) {
                            NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary];

                            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                            appManager->SetRecentPin([json[@"id"] intValue], [json[@"pinned"] boolValue]);
                        } else if (cmd.find(L"go:folder") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameEditorOpenFolder
                                                                                object:nil
                                                                              userInfo: @{
                                                                                          @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                          @"path": [NSString stringWithstdwstring:param]
                                                                                          }];
                        } else if (cmd.find(L"settings:get") != std::wstring::npos) {
                            if (param.find(L"username") != std::wstring::npos) {
                                if (NSString * fullUserName = NSFullUserName()) {
                                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                    pCommand->put_Command(L"settings:username");
                                    pCommand->put_Param([fullUserName stdwstring]);

                                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                                    pEvent->m_pData = pCommand;

                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    appManager->SetEventToAllMainWindows(pEvent);
                                }
                            }
                        } else if (cmd.find(L"settings:apply") != std::wstring::npos) {
                            if (NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary]) {
                                id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

                                if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
                                    [[ASCEditorJSVariables instance] setParameter:@"lang" withString:[externalDelegate onAppPreferredLanguage]];
                                } else {
                                    [[ASCEditorJSVariables instance] setParameter:@"lang" withString:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];
                                }
                                
                                if (NSString * langId = json[@"langid"]) {
                                    if ( [ASCLinguist appLanguageCode] != langId ) {
                                        [ASCLinguist setAppLanguageCode:langId];

                                        NSMutableDictionary * json = [[NSMutableDictionary alloc] initWithDictionary: @{@"lang": langId}];
                                        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                        appManager->UpdatePlugins([[json jsonString] stdwstring]);
                                    }
                                }

                                if (NSString * userName = json[@"username"]) {
                                    if ([userName isEqualToString:@""]) {
                                        [[ASCEditorJSVariables instance] setParameter:@"username" withString:NSFullUserName()];
                                        [[NSUserDefaults standardUserDefaults] setObject:NSFullUserName() forKey:ASCUserNameApp];
                                    } else {
                                        [[ASCEditorJSVariables instance] setParameter:@"username" withString:userName];
                                        [[NSUserDefaults standardUserDefaults] setObject:userName forKey:ASCUserNameApp];
                                    }
                                    [[NSUserDefaults standardUserDefaults] synchronize];
                                } else {
                                    [[ASCEditorJSVariables instance] setParameter:@"username" withString:NSFullUserName()];
                                }

                                if (NSString * docopenMode = json[@"docopenmode"]) {
                                    if ([docopenMode isEqualToString:@"view"]) {
                                        [[ASCEditorJSVariables instance] setParameter:@"mode" withString:@"view"];
                                    } else {
                                        [[ASCEditorJSVariables instance] removeParameter:@"mode"];
                                    }
                                    [[NSUserDefaults standardUserDefaults] setObject:docopenMode forKey:@"asc_user_docOpenMode"];
                                    [[NSUserDefaults standardUserDefaults] synchronize];
                                }

                                if (NSString * uiTheme = json[@"uitheme"]) {
                                    if ( ![uiTheme isEqualToString:[[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme]] ) {
                                        [[NSUserDefaults standardUserDefaults] setObject:uiTheme forKey:ASCUserUITheme];

                                        [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameChangedUITheme
                                                                                            object:nil
                                                                                          userInfo: @{@"uitheme": uiTheme}];
                                    }

                                    [[ASCEditorJSVariables instance] setParameter:@"uitheme" withString:uiTheme];
                                }

                                if ( [json objectForKey:@"usegpu"] != nil ) {
                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    appManager->GetUserSettings()->Set(L"disable-gpu", [json[@"usegpu"] boolValue] ? L"0" : L"1");
                                }

                                if ( [json objectForKey:@"useai"] != nil ) {
                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    appManager->GetUserSettings()->Set(L"disable-ai", [json[@"useai"] boolValue] ? L"0" : L"1");
                                }

                                if ( [json objectForKey:@"editorwindowmode"] != nil ) {
                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    appManager->GetUserSettings()->Set(L"editor-window-mode", [json[@"editorwindowmode"] boolValue] ? L"1" : L"0");
                                }

                                if ( [json objectForKey:@"spellcheckdetect"] != nil ) {
                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    appManager->GetUserSettings()->Set(L"spell-check-input-mode", [json[@"spellcheckdetect"] isEqualToString:@"off"] ? L"0" : L"1");
                                }

                                [[ASCEditorJSVariables instance] applyParameters];
                            }
                        } else if (cmd.find(L"encrypt:isneedbuild") != std::wstring::npos) {
                            bool isFragmented = pData->get_Param() == L"true" ? true : false;
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameDocumentFragmented
                                                                                object:nil
                                                                              userInfo: @{
                                                                                          @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                          @"isFragmented": @(isFragmented)
                                                                                          }];
                        } else if (cmd.find(L"create:new") != std::wstring::npos) {
                            /// Create local files
                            
                            NSString * nsParam = (NSString *)[NSString stringWithstdwstring:param];
                            AscEditorType docType = AscEditorType::etUndefined;
                            
                            if ([nsParam hasPrefix:@"template:"]) {
                                if ([nsParam hasSuffix:@"word"]) {
                                    docType = AscEditorType::etDocument;
                                } else
                                if ([nsParam hasSuffix:@"slide"]) {
                                    docType = AscEditorType::etPresentation;
                                } else
                                if ([nsParam hasSuffix:@"cell"]) {
                                    docType = AscEditorType::etSpreadsheet;
                                }

                                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                    object:nil
                                                                                  userInfo:@{
                                                                                      @"action"  : @(ASCTabActionCreateLocalFileFromTemplate),
                                                                                      @"type"    : @(int(docType)),
                                                                                      @"active"  : @(YES)
                                                                                  }];
                            } else if ([nsParam hasPrefix:@"{\"template\":"]) {
                                NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary][@"template"];

                                AscEditorType docType = AscEditorType::etDocument;
                                if ( [json objectForKey:@"type"] ) {
                                    int tplType = [json[@"type"] intValue];
//                                    if ( tplType > AVS_OFFICESTUDIO_FILE_DOCUMENT and tplType < AVS_OFFICESTUDIO_FILE_PRESENTATION ) docType = AscEditorType::etDocument; else
                                    if ( tplType > AVS_OFFICESTUDIO_FILE_PRESENTATION and tplType < AVS_OFFICESTUDIO_FILE_SPREADSHEET ) docType = AscEditorType::etPresentation; else
                                    if ( tplType > AVS_OFFICESTUDIO_FILE_SPREADSHEET and tplType < AVS_OFFICESTUDIO_FILE_CROSSPLATFORM ) docType = AscEditorType::etSpreadsheet; else
                                    if ( tplType == AVS_OFFICESTUDIO_FILE_DOCUMENT_OFORM_PDF ) { docType = AscEditorType::etPdf; }
//                                    else if ( tplType > AVS_OFFICESTUDIO_FILE_CROSSPLATFORM and tplType < AVS_OFFICESTUDIO_FILE_IMAGE ) {}
                                }

                                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                    object:nil
                                                                                  userInfo:@{
                                                                                        @"action"  : @(ASCTabActionCreateLocalFileFromTemplate),
                                                                                        @"type"    : @(int(docType)),
                                                                                        @"active"  : @(YES),
                                                                                        @"path"    : json[@"path"],
                                                                                        @"id"      : json[@"id"],
                                                                                    }];
                            } else {
                                if ([nsParam isEqualToString:@"word"]) {
                                    docType = AscEditorType::etDocument;
                                } else if ([nsParam isEqualToString:@"cell"]) {
                                    docType = AscEditorType::etSpreadsheet;
                                } else if ([nsParam isEqualToString:@"slide"]) {
                                    docType = AscEditorType::etPresentation;
                                } else if ([nsParam isEqualToString:@"form"]) {
                                    docType = AscEditorType::etDocumentMasterForm;
                                }

                                if ( docType != AscEditorType::etUndefined ) {
                                    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                        object:nil
                                                                                      userInfo:@{
                                                                                        @"action"  : @(ASCTabActionCreateLocalFile),
                                                                                        @"type"    : @(int(docType)),
                                                                                        @"active"  : @(YES)
                                    }];
                                }
                            }
                        } else if (cmd.find(L"open:folder") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenLocalFile
                                                                                object:nil
                                                                              userInfo:@{
                                                                                  @"directory": [NSString stringWithstdwstring:param]
                                                                              }];
                        } else if (cmd.find(L"extra:features") != std::wstring::npos) {
                            if (NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary]) {
                                if (NSArray * available = json[@"available"]) {
                                    [[ASCSharedSettings sharedInstance] setSetting:@([available count] > 0) forKey:kSettingsHasExtraFeatures];
                                }
                            }
                        } else if (cmd.find(L"open:document") != std::wstring::npos) {
                            if (!param.empty()) {
                                if (param.rfind(L"https://",0) == 0 || param.rfind(L"http://",0) == 0) {
                                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                                    CCefView * pCefView = appManager->GetViewByUrl(param);
                                    int viewId = pCefView ? pCefView->GetId() : -1;

                                    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                        object:nil
                                                                                      userInfo:@{
                                                                                             @"action"  : @(ASCTabActionOpenUrl),
                                                                                             @"viewId"  : [NSString stringWithFormat:@"%d", viewId],
                                                                                             @"url"     : [NSString stringWithstdwstring:param],
                                                                                             @"active"  : @(true)
                                                                                             }];
                                }
                            }
                        } else if (cmd.find(L"open:recent") != std::wstring::npos) {
                            if (NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary]) {
                                bool fromRecovery  = [json objectForKey:@"recovery"] && [json[@"recovery"] boolValue];
                                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                                    object:nil
                                                                                  userInfo:@{
                                                                                             @"action"  : !fromRecovery ? @(ASCTabActionOpenLocalRecentFile) : @(ASCTabActionOpenLocalRecoverFile),
                                                                                             @"active"  : @(YES),
                                                                                             @"fileId"  : json[@"id"],
                                                                                             @"path"    : json[@"path"]
                                                                                             }];
                            }
                        } else if (cmd.find(L"recovery:update") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameRecoveryFiles
                                                                                object:nil
                                                                                userInfo:@{
                                                                                        @"files":[NSString stringWithstdwstring:param]
                                                                                }];
                        } else if (cmd.find(L"webapps:features") != std::wstring::npos) {
                            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                            CCefView * pCefView = appManager->GetViewById(senderId);

                            if (pCefView) {
                                BOOL isDark = [ASCThemesController isCurrentThemeDark];
                                NSString * uiTheme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme] ?:
                                                        [ASCThemesController defaultThemeId:isDark];

                                NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                pCommand->put_FrameName(L"frameEditor");
                                pCommand->put_Command(L"uitheme:changed");
                                pCommand->put_Param([uiTheme stdwstring]);

                                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                                pEvent->m_pData = pCommand;

                                pCefView->Apply(pEvent);
                            }
                        } else if (cmd.find(L"system:changed") != std::wstring::npos) {
//                            NSLog(@"nstheme: system changed %@", [NSString stringWithstdwstring:param]);
//                            if ( [[ASCThemesController currentThemeId] isEqualToString:uiThemeSystem] ) {
//                                NSError * error = NULL;
//                                NSRegularExpression * regex = [NSRegularExpression regularExpressionWithPattern: @":\\s?\\\"(light|dark)"
//                                                                                                       options: NSRegularExpressionCaseInsensitive
//                                                                                                         error: &error];
//                                if ( !error ) {
//                                    NSString * json = [NSString stringWithstdwstring:param];
//                                    NSTextCheckingResult * match = [regex firstMatchInString:json options:0 range:NSMakeRange(0, [json length])];
//                                    if (match) {
//                                        NSRange range = [match rangeAtIndex:1];
//                                        NSString * new_theme_type = [json substringWithRange:range];
//
//                                        if ( [ASCThemesController isCurrentThemeDark] != [new_theme_type isEqualToString:@"dark"] ) {
//                                            [[ASCSharedSettings sharedInstance] setSetting:new_theme_type forKey:kSettingsColorScheme];
//                                            [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameChangedUITheme
//                                                                                                object:nil
//                                                                                              userInfo:@{@"uitheme": uiThemeSystem}];
//                                        }
//                                    }
//                                }
//                            } else {
//                                if (NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary]) {
//                                    if ( NSString * colors = json[@"colorscheme"] ) {
//                                        [[ASCSharedSettings sharedInstance] setSetting:colors forKey:kSettingsColorScheme];
//                                    }
//                                }
//                            }
                        } else if (cmd.find(L"uitheme:changed") != std::wstring::npos) {
                            if (NSDictionary * json = [[NSString stringWithstdwstring:param] dictionary]) {
                                if ( NSString * newTheme = json[@"name"] ) {
                                    NSString * curTheme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];

                                    if ( ![curTheme isEqualToString:newTheme] ) {
                                        [[NSUserDefaults standardUserDefaults] setObject:newTheme forKey:ASCUserUITheme];

                                        [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameChangedUITheme
                                                                                            object:nil
                                                                                          userInfo:@{@"uitheme":newTheme}];
//                                    } else if ( [curTheme isEqualToString:uiThemeSystem] ) {
//                                        NSString * colorScheme = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsColorScheme];
//                                        if ( [NSApplication isSystemDarkMode] != [colorScheme isEqualToString:@"dark"] ) {
//                                            [[ASCSharedSettings sharedInstance] setSetting:([NSApplication isSystemDarkMode] ? @"dark" : @"light") forKey:kSettingsColorScheme];
//
//                                            [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameChangedUITheme
//                                                                                                object:nil
//                                                                                              userInfo:@{@"uitheme":newTheme}];
//                                        }
                                    }
                                }
                            }
                        } else if (cmd.find(L"webapps:entry") != std::wstring::npos) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameWebAppsEntry
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                         @"info": [NSString stringWithstdwstring:param]
                                                                                         }];
                        } else if (cmd.compare(L"title:button") == 0 ) {
                            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameWebTitleChanged
                                                                                object:nil
                                                                              userInfo:@{
                                                                                         @"viewId": [NSString stringWithFormat:@"%d", senderId],
                                                                                         @"info": [NSString stringWithstdwstring:param]
                                                                                         }];
                        }

                        break;
                    }

                    default:
                        break;
                }

                if (externalDelegate && [externalDelegate respondsToSelector:@selector(onCefMenuEvent:)]) {
                    [externalDelegate onCefMenuEvent:pEvent];
                }
                
                if (NULL != pEvent) {
                    pEvent->Release();
                }
            });
        });
    }
    
    virtual bool IsSupportEvent(int nEventType)
    {
        return true;
    }
    
    void resetFullscreenState()
    {
        fullscreen_view_id = -1;
    }
};


#pragma mark -
#pragma mark ========================================================
#pragma mark ASCEventsController
#pragma mark ========================================================
#pragma mark -

@interface ASCEventsController()
@property (nonatomic) ASCEventListener *listener;
@end

@implementation ASCEventsController

+ (instancetype)sharedInstance
{
    static id sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    
    return sharedInstance;
}

- (id)init {
    self = [super init];
    
    if (self) {
        _listener = new ASCEventListener();
        
        if (_listener) {
            CAscApplicationManager *appManager = [NSAscApplicationWorker getAppManager];
            
            if (appManager) {
                appManager->SetEventListener(_listener);
            }
        }
    }
    
    return self;
}

+ (void)resetFullscreenState {
    ASCEventsController *ec = [ASCEventsController sharedInstance];
    if (ec && ec->_listener) {
        ec->_listener->resetFullscreenState();
    }
}

@end

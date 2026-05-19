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

//
//  main.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "mac_application.h"
#include "ASCApplicationManager.h"
#import "NSString+Extensions.h"
#import "ASCConstants.h"
#import "ASCHelper.h"
#import "ASCLinguist.h"
#import "ASCDocSignController.h"
#import "ASCExternalController.h"
#import "NSApplication+Extensions.h"
#import "NSDictionary+Extensions.h"
#import "ASCEditorJSVariables.h"
#import "ASCSharedSettings.h"
#import "ASCEventsController.h"
#import "ASCThemesController.h"

CAscApplicationManager * createASCApplicationManager() {
    return new ASCApplicationManager();
}

int main(int argc, const char * argv[]) {
//    return NSApplicationMain(argc, argv);
    
    [ASCHelper createCloudPath];
    [ASCLinguist init];

    NSAscApplicationWorker * worker = [[NSAscApplicationWorker alloc] initWithCreator:createASCApplicationManager];
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];

    // setup common user directory
    appManager->m_oSettings.SetUserDataPath([[ASCHelper applicationDataPath] stdwstring]);
    
    NSString * resourcePath = [[NSBundle mainBundle] resourcePath];
    // setup Editors directory
    appManager->m_oSettings.local_editors_path = [[resourcePath stringByAppendingPathComponent:@"editors/web-apps/apps/api/documents/index.html"] stdwstring];
    
    appManager->m_oSettings.system_plugins_path = [[resourcePath stringByAppendingPathComponent:@"editors/sdkjs-plugins"] stdwstring];
    
    // setup Dictionary directory
    appManager->m_oSettings.spell_dictionaries_path = [[resourcePath stringByAppendingPathComponent:@"dictionaries"] stdwstring];
    
    // setup Recovery directory
    appManager->m_oSettings.recover_path = [[ASCHelper recoveryDataPath] stdwstring];
    
    // setup Converter directory
    appManager->m_oSettings.file_converter_path = [[resourcePath stringByAppendingPathComponent:@"converter"] stdwstring];
    appManager->m_oSettings.system_templates_path = [[resourcePath stringByAppendingPathComponent:@"converter/templates"] stdwstring];
    
    // setup editor fonts directory
    std::vector<std::wstring> fontsDirectories;
    fontsDirectories.push_back([[resourcePath stringByAppendingPathComponent:@"login/fonts"] stdwstring]);
    appManager->m_oSettings.additional_fonts_folder = fontsDirectories;
    
    NSString * error_page = [resourcePath stringByAppendingPathComponent:@"login/noconnect.html"];
    if( [[NSFileManager defaultManager] fileExistsAtPath:error_page] ) {
        appManager->m_oSettings.connection_error_path = [error_page stdwstring];
    }
    
    // setup username
    NSString * fullName = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserNameApp];
    if (fullName == nil) {
        fullName = NSFullUserName();
    }
    
    if (fullName) {
        [[ASCEditorJSVariables instance] setParameter:@"username" withString:fullName];
    }
    
    // setup ui theme
    [ASCThemesController sharedInstance];

    [[ASCEditorJSVariables instance] setVariable:@"rtl" withBool:[ASCLinguist isUILayoutDirectionRtl]];
    [[ASCEditorJSVariables instance] apply];

    // setup doc sign
    [ASCDocSignController shared];
    
    // Create CEF event listener
    [ASCEventsController sharedInstance];

    //[worker Start:argc :argv];
    [worker Start:argc argv:argv];
    int result = NSApplicationMain(argc, argv);
    [worker End];
    return result;
}

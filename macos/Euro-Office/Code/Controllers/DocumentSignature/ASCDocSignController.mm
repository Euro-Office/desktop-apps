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
//  ASCDocSignController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 17/04/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import "ASCDocSignController.h"
#import "NSString+Extensions.h"
#import "mac_application.h"
#import "ASCConstants.h"
#import "ASCDocumentSignatureController.h"


@implementation ASCDocSignController

#pragma mark - Life Cycle Methods

+ (instancetype)shared {
    static ASCDocSignController * sharedManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedManager = [[self alloc] init];
        [sharedManager initialize];
    });
    return sharedManager;
}

- (void)internalReset {
    self.signFilePath = @"";
    self.signPassword = @"";
    self.privateKeyFilePath = @"";
    self.privateKeyPassword = @"";

    _cefId = -1;
}

- (void)initialize {
    [self internalReset];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFOpenSSLCertificate:)
                                                 name:CEFEventNameOpenSSLCertificate
                                               object:nil];
}

- (void)onCEFOpenSSLCertificate:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        if (NSString * viewId = json[@"viewId"]) {
            [[ASCDocSignController shared] startWizardWithCefId:viewId];
        }
    }
}

- (void)startWizardWithCefId:(NSString *)cefId {
    [self internalReset];

    if (cefId && cefId.length > 0) {
        _cefId = [cefId integerValue];
    }

    NSStoryboard * storyboard = [NSStoryboard storyboardWithName:StoryboardNameSign bundle:[NSBundle mainBundle]];

    if (storyboard) {
        NSWindowController * windowController = [storyboard instantiateControllerWithIdentifier:@"DocSignWindowController"];
        NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];

        [mainWindow beginSheet:[windowController window] completionHandler:^(NSModalResponse returnCode) {
//            [NSApp stopModalWithCode:returnCode]; // Revert if need synchronize
        }];

//        [NSApp runModalForWindow:[windowController window]]; // Revert if need synchronize
    }
}

@end

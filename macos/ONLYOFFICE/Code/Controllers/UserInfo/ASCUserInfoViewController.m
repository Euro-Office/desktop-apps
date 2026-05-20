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
//  ASCUserInfoViewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/11/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCUserInfoViewController.h"
#import "ASCHelper.h"
#import "ASCConstants.h"
#import "GTMNSString+HTML.h"

@interface ASCUserInfoViewController ()

@end

@implementation ASCUserInfoViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFLogin:)
                                                 name:CEFEventNameLogin
                                               object:nil];

    NSDictionary * userInfo = [[ASCHelper localSettings] valueForKey:ASCUserSettingsNameUserInfo];

    if (userInfo) {
        [self.userNameText setStringValue:[userInfo[@"user"][@"displayName"] gtm_stringByUnescapingFromHTML]];
        [self.portalText setStringValue:[userInfo[@"portal"] gtm_stringByUnescapingFromHTML]];
        [self.emailText setStringValue:userInfo[@"user"][@"email"]];
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark -
#pragma mark - CEF events handler

- (void)onCEFLogin:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * userInfo = (NSDictionary *)notification.userInfo;
        
        [[ASCHelper localSettings] setValue:userInfo forKey:ASCUserSettingsNameUserInfo];
        
        if (userInfo) {
            [self.userNameText setStringValue:[userInfo[@"user"][@"displayName"] gtm_stringByUnescapingFromHTML]];
            [self.portalText setStringValue:[userInfo[@"portal"] gtm_stringByUnescapingFromHTML]];
            [self.emailText setStringValue:userInfo[@"user"][@"email"]];
        }
    }
}

#pragma mark -
#pragma mark - Actions

- (IBAction)onLogoutButton:(NSButton *)sender {
    if (_delegate && [_delegate respondsToSelector:@selector(onLogoutButton:)]) {
        [_delegate onLogoutButton:self];
    }
    
    if (self.popover) {
        [self.popover closePopover:sender];
    }
}

@end

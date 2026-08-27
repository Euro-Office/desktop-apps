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

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
//  ASCTabViewController.m
//  NSViewControllerPresentations
//
//  Created by Alexander Yuzhin on 12/26/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTabViewController.h"

@interface ASCTabViewController ()

@end

@implementation ASCTabViewController

- (void)awakeFromNib {
    NSTabViewItem *tabViewItem = self.tabView.selectedTabViewItem;
    self.view.window.title = tabViewItem.viewController.title;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.tabView.delegate = self;
    
    for (NSTabViewItem * tabViewItem in self.tabView.tabViewItems) {
        [tabViewItem.viewController prepareForSegue:[[NSStoryboardSegue alloc] init] sender:self];
    }
}


#pragma mark -
#pragma mark NSTabViewDelegate Method

- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem {
    [super tabView:tabView didSelectTabViewItem:tabViewItem];
    
    tabView.hidden = YES;
    
    NSWindow *window = self.view.window;
    NSRect oldWindowFrame = window.frame;
    window.title = tabViewItem.viewController.title;
    
    NSRect viewFrame = tabViewItem.view.frame;
    viewFrame.size = tabViewItem.view.fittingSize;
    
    NSArray *constraints = tabViewItem.view.constraints;
    [tabViewItem.view removeConstraints:constraints];
    
    NSRect windowFrame = [window frameRectForContentRect:viewFrame];
    windowFrame.origin = NSMakePoint(window.frame.origin.x + (NSWidth(oldWindowFrame) - NSWidth(windowFrame)) * 0.5, NSMaxY(window.frame) - NSHeight(windowFrame));
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [[window animator] setFrame:windowFrame display:YES];
    } completionHandler:^{
        [tabViewItem.view addConstraints:constraints];
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [[tabView animator] setHidden:NO];
            [tabViewItem.viewController prepareForSegue:[[NSStoryboardSegue alloc] init] sender:self];
        } completionHandler:NULL];
    }];
}
@end

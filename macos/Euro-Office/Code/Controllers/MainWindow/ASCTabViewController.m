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

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
//  ASCTitleWindowController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/8/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTitleWindowController.h"
#import "ASCTitleWindow.h"
#import "ASCConstants.h"
#import "ASCCommonViewController.h"
#import "ASCHelper.h"

@interface ASCTitleWindowController ()
@property (nonatomic, assign) NSRect savedNormalFrame;
@end

@implementation ASCTitleWindowController

- (void)windowDidLoad {
    NSString * productName = [ASCHelper appName];

    self.window.title = productName;
    
    [super windowDidLoad];
    self.savedNormalFrame = [self.window frame];

    [self.window setFrameAutosaveName:@"MainWindow"];
    [self setShouldCascadeWindows:NO];
    
    [self setupToolbar];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameMainWindowLoaded
                                                        object:self];
}

- (BOOL)shouldPerformClose {
    ASCTitleWindow * window = (ASCTitleWindow *)self.window;
    ASCCommonViewController * controller = (ASCCommonViewController *)window.contentViewController;
    return [controller shouldCloseWindowIfNoTabs];
}

- (BOOL)windowShouldClose:(id)sender {
    ASCTitleWindow * window = (ASCTitleWindow *)self.window;
    ASCCommonViewController * controller = (ASCCommonViewController *)window.contentViewController;
//    NSEvent * event = [window currentEvent];

//    BOOL isCommandKey = ([event modifierFlags] & NSCommandKeyMask) != 0;

//    if (event != nil && ((event.type != NSEventTypeLeftMouseUp) || event.window == nil)) {
//        if (![controller shouldCloseWindowIfNoTabs]) {
//            return NO;
//        }
//    }
    
    return [controller shouldCloseWindow];
}

- (BOOL)windowShouldZoom:(NSWindow *)window toFrame:(NSRect)newFrame {
    if ( ![window isZoomed] ) {
        self.savedNormalFrame = [window frame];
    }
    return YES;
}

- (void)windowWillEnterFullScreen:(NSNotification *)notification {
    NSWindow *window = notification.object;
    if ( ![window isZoomed] ) {
        self.savedNormalFrame = [window frame];
    }
}

- (NSRect)normalFrame {
    ASCTitleWindow *window = (ASCTitleWindow *)self.window;
    BOOL isFullScreen = ([window styleMask] & NSWindowStyleMaskFullScreen) != 0;
    NSRect frame;
    if ( [window isZoomed] || isFullScreen ) {
        frame = self.savedNormalFrame;
    } else {
        frame = [window frame];
    }
    return frame;
}

- (float)defaultTitleBarHeight {
    NSRect frame = NSMakeRect(0, 0, 800, 600);
    NSRect contentRect = [NSWindow contentRectForFrameRect:frame styleMask: NSWindowStyleMaskTitled];
    return NSHeight(frame) - NSHeight(contentRect);
}

- (void)setupToolbar {
    ASCTitleWindow *window = (ASCTitleWindow *)self.window;
//    window.titleBarHeight = 30 - defaultTitleBarHeight();
    
    self.titlebarController = [self.storyboard instantiateControllerWithIdentifier:@"titleBarControllerID"];

    NSView * titlebar = [[window standardWindowButton:NSWindowCloseButton] superview];
    [titlebar addSubview:self.titlebarController.view];
    
    NSView * view = self.titlebarController.view;
    NSView * superview = view.superview;
    
    view.translatesAutoresizingMaskIntoConstraints = NO;

    // leading
    [superview addConstraint:[NSLayoutConstraint constraintWithItem:view
                                                          attribute:NSLayoutAttributeLeading
                                                          relatedBy:NSLayoutRelationEqual
                                                             toItem:superview
                                                          attribute:NSLayoutAttributeLeading
                                                         multiplier:1
                                                           constant:0]];
    // top
    [superview addConstraint:[NSLayoutConstraint constraintWithItem:view
                                                          attribute:NSLayoutAttributeTop
                                                          relatedBy:NSLayoutRelationEqual
                                                             toItem:superview
                                                          attribute:NSLayoutAttributeTop
                                                         multiplier:1
                                                           constant:0]];
    // width
//    [superview addConstraint:[NSLayoutConstraint constraintWithItem:superview
//                                                          attribute:NSLayoutAttributeWidth
//                                                          relatedBy:NSLayoutRelationEqual
//                                                             toItem:view attribute:NSLayoutAttributeWidth
//                                                         multiplier:1
//                                                           constant:0]];
    // width
    [superview addConstraint:[NSLayoutConstraint constraintWithItem:superview
                                                          attribute:NSLayoutAttributeTrailing
                                                          relatedBy:NSLayoutRelationEqual
                                                             toItem:view attribute:NSLayoutAttributeTrailing
                                                         multiplier:1
                                                           constant:0]];
    // height
    [superview addConstraint:[NSLayoutConstraint constraintWithItem:superview
                                                          attribute:NSLayoutAttributeHeight
                                                          relatedBy:NSLayoutRelationEqual
                                                             toItem:view
                                                          attribute:NSLayoutAttributeHeight
                                                         multiplier:1
                                                           constant:0]];
}


@end

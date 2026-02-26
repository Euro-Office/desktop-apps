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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
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

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
//  ASCEditorTitleBarController.m
//  ONLYOFFICE
//
//  Copyright (c) 2025 Ascensio System SIA. All rights reserved.
//

#import "ASCEditorTitleBarController.h"
#import "ASCEditorWindow.h"
#import "ASCConstants.h"
#import "ASCLinguist.h"
#import "NSView+Extensions.h"

static float kASCEditorWindowDefaultTrafficButtonsLeftMargin = 0;

@interface ASCEditorTitleBarController()
@property (nonatomic, weak) NSWindow *editorWindow;
@property (nonatomic) NSArray *standardButtonsDefaults;
@property (nonatomic) NSArray *standardButtonsFullscreen;
@property (nonatomic, weak) NSButton *closeButtonFullscreen;
@property (nonatomic, weak) NSButton *miniaturizeButtonFullscreen;
@property (nonatomic, weak) NSButton *fullscreenButtonFullscreen;
@end

@implementation ASCEditorTitleBarController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)setupWithWindow:(NSWindow *)editorWindow {
    self.editorWindow = editorWindow;

    self.closeButtonFullscreen = [NSWindow standardWindowButton:NSWindowCloseButton forStyleMask:NSWindowStyleMaskTitled];
    self.fullscreenButtonFullscreen = [NSWindow standardWindowButton:NSWindowZoomButton forStyleMask:NSWindowStyleMaskTitled];
    self.miniaturizeButtonFullscreen = [NSWindow standardWindowButton:NSWindowMiniaturizeButton forStyleMask:NSWindowStyleMaskTitled];
    self.miniaturizeButtonFullscreen.enabled = NO;

    self.standardButtonsFullscreen = @[self.closeButtonFullscreen, self.miniaturizeButtonFullscreen, self.fullscreenButtonFullscreen];
    if ([self.view userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft) {
        self.standardButtonsFullscreen = [[self.standardButtonsFullscreen reverseObjectEnumerator] allObjects];
    }

    [self.standardButtonsFullscreen enumerateObjectsUsingBlock:^(NSView *standardButtonView, NSUInteger idx, BOOL *stop) {
        [self.view addSubview:standardButtonView];
    }];

    self.standardButtonsDefaults = @[[editorWindow standardWindowButton:NSWindowCloseButton],
                                     [editorWindow standardWindowButton:NSWindowMiniaturizeButton],
                                     [editorWindow standardWindowButton:NSWindowZoomButton]];

    if ([self.view userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft) {
        self.standardButtonsDefaults = [[self.standardButtonsDefaults reverseObjectEnumerator] allObjects];
    }

    [self.standardButtonsDefaults enumerateObjectsUsingBlock:^(NSButton *standardButton, NSUInteger idx, BOOL *stop) {
        [self.view addSubview:standardButton];
    }];

    kASCEditorWindowDefaultTrafficButtonsLeftMargin = NSWidth(self.closeButtonFullscreen.frame) - 2.0;

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowDidResize:)
                                                 name:NSWindowDidResizeNotification
                                               object:editorWindow];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onWindowSetFrame:)
                                                 name:ASCEventNameEditorWindowSetFrame
                                               object:editorWindow];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowDidBecomeKey:)
                                                 name:NSWindowDidBecomeKeyNotification
                                               object:editorWindow];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowDidResignKey:)
                                                 name:NSWindowDidResignKeyNotification
                                               object:editorWindow];

    [self doLayout];
}

- (BOOL)isFullScreen {
    NSWindow *editorWindow = self.editorWindow;
    if (!editorWindow) return NO;
    return (([editorWindow styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask);
}

- (void)doLayout {
    NSWindow *editorWindow = self.editorWindow;
    if (!editorWindow) return;
    
    int btnSpacing = 6.0;
    CGFloat btnContainerWidth = CGRectGetWidth([self.standardButtonsDefaults[0] frame]) + btnSpacing;
    CGFloat leftOffsetForTrafficLightButtons = kASCEditorWindowDefaultTrafficButtonsLeftMargin;
    
    if ( [ASCLinguist isUILayoutDirectionRtl] ) {
        CGFloat windowWidth = CGRectGetWidth([editorWindow frame]);
        leftOffsetForTrafficLightButtons = windowWidth - kASCEditorWindowDefaultTrafficButtonsLeftMargin - btnContainerWidth * 3 + btnSpacing;
    }
    
    void (^layoutStandartButtons)(NSArray *, BOOL) = ^(NSArray *views, BOOL hidden) {
        [views enumerateObjectsUsingBlock:^(NSView *view, NSUInteger idx, BOOL *stop) {
            NSRect frame = view.frame;
            frame.origin.x = leftOffsetForTrafficLightButtons + idx * btnContainerWidth;
            frame.origin.y = (int)((NSHeight(view.superview.frame) - NSHeight(view.frame)) / 2.0);
            
            [view setFrame:frame];
            [view setHidden:hidden];
            [view setNeedsDisplay:YES];
        }];
    };
    
    layoutStandartButtons(self.standardButtonsDefaults, [self isFullScreen]);
    layoutStandartButtons(self.standardButtonsFullscreen, ![self isFullScreen]);
}

- (void)windowDidResize:(NSNotification *)notification {
    [self doLayout];
}

- (void)onWindowSetFrame:(NSNotification *)notification {
    [self doLayout];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    [self updateWindowButtonsAppearance];
}

- (void)windowDidResignKey:(NSNotification *)notification {
    [self updateWindowButtonsAppearance];
}

- (void)updateWindowButtonsAppearance {
    [self.standardButtonsFullscreen enumerateObjectsUsingBlock:^(NSView *view, NSUInteger idx, BOOL *stop) {
        [view setNeedsDisplay:YES];
    }];
    [self.standardButtonsDefaults enumerateObjectsUsingBlock:^(NSView *view, NSUInteger idx, BOOL *stop) {
        [view setNeedsDisplay:YES];
    }];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end

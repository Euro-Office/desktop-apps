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
//  ASCPresentationReporter.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 10/16/17.
//  Copyright © 2017 Ascensio System SIA. All rights reserved.
//

#import "ASCPresentationReporter.h"
#import "NSCefView.h"
#import "mac_application.h"
#import "NSView+Extensions.h"
#import "PureLayout.h"
#import "ASCConstants.h"
#import "ASCHelper.h"
#import "ASCCommonViewController.h"
#import "ASCEditorWindowController.h"
#import "ASCTitleWindow.h"
#import "ASCEditorWindow.h"

@interface ASCPresentationReporter() <NSWindowDelegate>
@property (nonatomic) NSStoryboard * storyboard;
@property (nonatomic) BOOL isDisplay;
@property (nonatomic) NSWindowController * controller;
@end

@implementation ASCPresentationReporter

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
        _storyboard = [NSStoryboard storyboardWithName:StoryboardNameReporter bundle:[NSBundle mainBundle]];
        _isDisplay = false;
    }
    
    return self;
}

- (void)create:(void *)data from:(int)senderId {
    if (_isDisplay || !_storyboard) {
        return;
    }
    
    _controller = [_storyboard instantiateControllerWithIdentifier:@"PresentationWindowController"];
    
    if (_controller) {
        _isDisplay = true;
        
        NSString * windowTitle = NSLocalizedString(@"Presenter View", nil);
        NSString * viewId = [NSString stringWithFormat:@"%d", senderId];
        
        for (NSWindow *window in [NSApp windows]) {
            if ([window isKindOfClass:[ASCTitleWindow class]]) {
                ASCCommonViewController * controller = (ASCCommonViewController *)window.contentViewController;
                ASCTabView * tab = [controller.tabsControl tabWithUUID:viewId];
                if (tab) {
                    windowTitle = [NSString stringWithFormat:@"%@ - %@", windowTitle, tab.title];
                    break;
                }

            } else
            if ([window isKindOfClass:[ASCEditorWindow class]]) {
                ASCEditorWindow *editor = (ASCEditorWindow *)window;
                ASCEditorWindowController *controller = (ASCEditorWindowController *)editor.windowController;
                if (controller && [controller holdView:viewId]) {
                    NSCefView * cefView = (NSCefView *)editor.webView;
                    windowTitle = [NSString stringWithFormat:@"%@ - %@", windowTitle, [cefView.data title:YES]];
                    break;
                }
            }
        }

        [_controller.window setTitle:windowTitle];

        NSCefView * cefView = [[NSCefView alloc] initWithFrame:CGRectZero];
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        if (cefView && appManager) {
            _controller.window.delegate = self;
            
            [_controller.contentViewController.view addSubview:cefView];
            [cefView autoPinEdgesToSuperviewEdges];

            NSArray<NSScreen *> * screens = [NSScreen screens];

            if (0 /*[screens count] > 1*/) {
                for (NSScreen * screen in screens) {
                    if (screen != [NSScreen mainScreen]) {
                        NSRect screenRect = [screen frame];
                        NSRect windowRect = [_controller.window frame];
                        windowRect.origin.x = screenRect.origin.x + (screenRect.size.width - windowRect.size.width) / 2;
                        windowRect.origin.y = screenRect.origin.y + (screenRect.size.height - windowRect.size.height) / 2;
                        [_controller.window setFrame:windowRect display:YES];
                        [_controller.window setLevel: CGShieldingWindowLevel()];
                        [_controller.window makeKeyAndOrderFront:screen];
                        break;
                    }
                }
            } else {
                if ([screens count] == 1) {
                    [_controller.window setLevel: NSFloatingWindowLevel];
                }
                [_controller showWindow:nil];
            }
            
            NSEditorApi::CAscReporterCreate * pData = (NSEditorApi::CAscReporterCreate *)data;
            CAscReporterData * pCreateData = reinterpret_cast<CAscReporterData *>(pData->get_Data());
            pData->put_Data(NULL);
            
            [cefView createReporter:appManager data:pCreateData];
        }
    }
}

- (void)destroy {
    if (!_isDisplay || !_controller) {
        return;
    }
    
    [_controller.window performClose:nil];
    _isDisplay = false;
}

- (void)apply:(void *)event {
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    NSEditorApi::CAscMenuEvent * pEvent = (NSEditorApi::CAscMenuEvent *)event;
    NSEditorApi::CAscReporterMessage * pData = (NSEditorApi::CAscReporterMessage *)pEvent->m_pData;
    
    CCefView * cefView = appManager->GetViewById(pData->get_ReceiverId());
    
    if (cefView) {
        pEvent->AddRef();
        cefView->Apply(pEvent);
    }
}

- (NSCefView *)cefView {
    if (!_controller) {
        return nil;
    }
    
    for (NSView * view in _controller.contentViewController.view.subviews) {
        if ([view isKindOfClass:[NSCefView class]]) {
            return (NSCefView *)view;
        }
    }
    return nil;
}

// MARK: - NSWindow Delegate
- (BOOL)windowShouldClose:(NSWindow *)sender {
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    NSCefView * cefView = [self cefView];
    
    if (appManager && cefView) {
        appManager->DestroyCefView((int)[cefView uuid]);
    }
    
    return true;
}

- (BOOL)isVisible
{
    return _isDisplay;
}

@end

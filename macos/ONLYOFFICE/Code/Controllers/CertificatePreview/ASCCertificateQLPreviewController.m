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
//  ASCCertificateQLPreviewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 10.02.2021.
//  Copyright © 2021 Ascensio System SIA. All rights reserved.
//

#import <Quartz/Quartz.h>
#import "ASCCertificateQLPreviewController.h"
#import "ASCCertificateQLPreview.h"

@interface ASCCertificateQLPreviewController() <QLPreviewPanelDelegate, QLPreviewPanelDataSource>
@property (nonatomic) ASCCertificateQLPreview * item;
@property (nonatomic) QLPreviewPanel * panel;
@end

@implementation ASCCertificateQLPreviewController

- (void)previewBy:(NSURL *)fileUrl {
    // Set responder to the controller for QLPreviewPanelController methods
    NSResponder * aNextResponder = [[NSApp mainWindow] nextResponder];
    [[NSApp mainWindow] setNextResponder:self];
    [self setNextResponder:aNextResponder];
    
    _item = [[ASCCertificateQLPreview alloc] init:fileUrl];
    _panel = [QLPreviewPanel sharedPreviewPanel];
    
    NSTimer * timer = [NSTimer scheduledTimerWithTimeInterval:0
                                                       target:self
                                                     selector:@selector(openPanelInRunLoop)
                                                     userInfo:nil
                                                      repeats:false];
    [[NSRunLoop currentRunLoop] addTimer:timer
                                 forMode:NSModalPanelRunLoopMode];
}

- (BOOL)isOpened {
    return [QLPreviewPanel sharedPreviewPanelExists] && [[QLPreviewPanel sharedPreviewPanel] isVisible];
}

- (void)openPanelInRunLoop {
    [_panel updateController];
    if (![self isOpened]) {
        [_panel makeKeyAndOrderFront:nil];
    } else {
        [_panel setCurrentPreviewItemIndex:0];
    }
}

#pragma mark - QLPreviewPanelDelegate

- (BOOL)acceptsPreviewPanelControl:(QLPreviewPanel *)panel {
    return true;
}

- (void)beginPreviewPanelControl:(QLPreviewPanel *)panel {
    _panel.dataSource = self;
    _panel.delegate = self;
    
    [[NSApp mainWindow] setNextResponder:[self nextResponder]];
}

- (void)endPreviewPanelControl:(QLPreviewPanel *)panel {
    _panel.dataSource = nil;
    _panel.delegate = nil;
    
    [[NSApp mainWindow] setNextResponder:[self nextResponder]];
    
    [self.item cleanup];
}

#pragma mark - QLPreviewPanelDataSource

- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *)panel {
    return 1;
}

- (id<QLPreviewItem>)previewPanel:(QLPreviewPanel *)panel previewItemAtIndex:(NSInteger)index {
    return self.item;
}

@end

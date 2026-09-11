/*
 * (c) Copyright Ascensio System SIA 2010-2021
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

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
//  ASCDownloadViewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/29/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCDownloadViewController.h"
#import "ASCDownloadController.h"
#import "ASCDownloadCellView.h"
#import "ASCHeaderButton.h"
#import "GTMNSString+HTML.h"
#import "ASCThemesController.h"
#import "ASCConstants.h"
#import "SFBPopoverWindow.h"

static const int kRowHeight    = 70;
static const int kHeaderHeight = 48;
static const int kPopoverWidth = 300;
static const int kMaxRows      = 6;

@interface ASCDownloadViewController() <NSTableViewDelegate, NSTableViewDataSource, ASCDownloadControllerDelegate, ASCDownloadCellViewDelegate>
@property (weak) IBOutlet NSView      *headerView;
@property (weak) IBOutlet NSView      *separatorView;
@property (weak) IBOutlet NSTextField *downloadsLabel;
@property (weak) IBOutlet NSTableView *tableView;
@property (weak) IBOutlet NSScrollView *tableScrollView;
@property (weak) IBOutlet ASCHeaderButton *clearButton;

@property (nonatomic, strong) NSMutableArray<NSString *> *rowUUIDs;
@end

@implementation ASCDownloadViewController


- (void)viewDidLoad {
    _rowUUIDs = [NSMutableArray array];
    
    for (id dl in [[ASCDownloadController sharedInstance] downloads]) {
        NSString *uuid = dl[@"idx"];
        if (uuid) [_rowUUIDs addObject:uuid];
    }

    [[[ASCDownloadController sharedInstance] multicastDelegate] addDelegate:self];
    
    self.clearButton.title = NSLocalizedString(@"Clear", nil);
    self.downloadsLabel.stringValue = NSLocalizedString(@"Downloads", nil);
}

- (void)viewDidDisappear {
    [[[ASCDownloadController sharedInstance] multicastDelegate] removeDelegate:self];
}

- (void)viewWillAppear {
    [self applyThemeColors];
    [self updatePopoverSize];
    [self updateClearButton];
}

- (void)updatePopoverSize {
    NSInteger count  = _rowUUIDs.count;
    int  newHeight   = [self desiredContentHeight];
    BOOL needsScroll = (count > kMaxRows);

    self.tableScrollView.hasVerticalScroller = needsScroll;
    self.tableScrollView.verticalScrollElasticity = needsScroll ? NSScrollElasticityAutomatic : NSScrollElasticityNone;

    self.preferredContentSize = NSMakeSize(kPopoverWidth, newHeight);
    
    NSWindow *window = self.view.window;
    NSRect  winFrame  = window.frame;
    CGFloat oldHeight = self.view.frame.size.height;
    CGFloat yOverhead = winFrame.size.height - oldHeight;

    NSRect newFrame = self.view.frame;
    newFrame.size.height = (CGFloat)newHeight;
    [self.view setFrame:newFrame];
    
    CGFloat newWinH = (CGFloat)newHeight + yOverhead;
    CGFloat delta = winFrame.size.height - newWinH;
    [window setFrame:NSMakeRect(winFrame.origin.x, winFrame.origin.y + delta, winFrame.size.width, newWinH) display:YES];
}

- (int)desiredContentHeight {
    NSInteger count = _rowUUIDs.count;
    int minH = kRowHeight + kHeaderHeight;
    int maxH = kRowHeight * kMaxRows + kHeaderHeight;
    int rawH = kRowHeight * (int)count + kHeaderHeight;
    return MAX(minH, MIN(rawH, maxH));
}

- (void)applyThemeColors {
    NSString *currentTheme = [ASCThemesController currentThemeId];
    SFBPopoverWindow *popoverWindow = (SFBPopoverWindow *)self.view.window;

    NSColor *bgColor = [ASCThemesController color:downloadWidgetBackgroundColor forTheme:currentTheme];
    if (bgColor) {
        [popoverWindow setPopoverBackgroundColor:bgColor];
        self.view.wantsLayer = YES;
        self.view.layer.backgroundColor = bgColor.CGColor;
    }
    
    NSColor *borderColor = [ASCThemesController color:downloadWidgetBorderColor forTheme:currentTheme];
    if (borderColor && self.separatorView) {
        [popoverWindow setBorderColor:borderColor];
        self.separatorView.wantsLayer = YES;
        self.separatorView.layer.backgroundColor = borderColor.CGColor;
    }
    
    NSColor *buttonTextColor = [ASCThemesController color:downloadGhostButtonTextColor forTheme:currentTheme];
    if (buttonTextColor && self.clearButton) {
        if (@available(macOS 10.14, *)) {
            self.clearButton.contentTintColor = buttonTextColor;
        }
    }
}

- (void)configureCell:(ASCDownloadCellView *)cell withDownload:(id)download {
    BOOL isComplete = [download[@"complete"] boolValue];
    BOOL isCanceled = [download[@"canceled"] boolValue];

    if (isCanceled) {
        cell.infoTextField.stringValue = NSLocalizedString(@"Canceled", nil);
        [self configureButton:cell.cancelButton asIconNamed:@"icon-warning_normal"];
        
    } else
    if (isComplete) {
        cell.progress.hidden       = YES;
        cell.sizeLabel.hidden      = NO;
        cell.sizeLabel.stringValue = [self fileSizeStringForPath:download[@"filePath"]] ?: @"";
        cell.filePath              = download[@"filePath"];
        cell.infoTextField.stringValue = [download[@"filePath"] stringByDeletingLastPathComponent] ?: @"";
        [self configureButton:cell.cancelButton asIconNamed:@"icon-confirm_normal"];

    } else {
        cell.progress.doubleValue = [download[@"percent"] doubleValue];
        cell.infoTextField.stringValue = [NSString stringWithFormat:@"%.0f %@", [download[@"speed"] doubleValue], NSLocalizedString(@"kBps", nil)];
        [self configureButton:cell.cancelButton asTitle:NSLocalizedString(@"Cancel", nil)];
    }
}

- (NSString *)fileSizeStringForPath:(NSString *)path {
    NSDictionary *attrs = [[NSFileManager defaultManager] attributesOfItemAtPath:path error:nil];
    if (!attrs) return nil;
    long long size = [[attrs objectForKey:NSFileSize] longLongValue];
    return [NSByteCountFormatter stringFromByteCount:size countStyle:NSByteCountFormatterCountStyleFile];
}

- (void)configureButton:(NSButton *)btn asIconNamed:(NSString *)name {
    btn.image = [NSImage imageNamed:name];
    btn.title = @"";
    btn.imagePosition = NSImageOnly;
    btn.imageScaling = NSImageScaleProportionallyDown;
    btn.bordered = NO;
    btn.bezelStyle = NSBezelStyleRegularSquare;
}

- (void)configureButton:(NSButton *)btn asTitle:(NSString *)title {
    btn.image = nil;
    btn.title = title;
    btn.imagePosition = NSNoImage;
    btn.bordered = NO;
    btn.bezelStyle = NSBezelStyleRegularSquare;
}

- (void)updateClearButton {
    BOOL hasFinished = NO;
    for (id dl in [[ASCDownloadController sharedInstance] downloads]) {
        if ([dl[@"finished"] boolValue]) {
            hasFinished = YES;
            break;
        }
    }
    _clearButton.enabled = hasFinished;
}

- (IBAction)onClearButtonClicked:(id)sender {
    [[ASCDownloadController sharedInstance] clearFinished];
}

#pragma mark -
#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return _rowUUIDs.count;
}

#pragma mark -
#pragma mark NSTableView Delegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    if (row < 0 || row >= (NSInteger)_rowUUIDs.count) return nil;

    NSString *uuid = _rowUUIDs[row];
    id download = [[ASCDownloadController sharedInstance] downloadWithId:uuid];
    if (!download) return nil;

    ASCDownloadCellView * cellView  = [tableView makeViewWithIdentifier:@"ASCDownloadTableViewCellId" owner:self];
    cellView.textField.stringValue  = [download[@"name"] gtm_stringByUnescapingFromHTML];
    cellView.uuid                   = download[@"idx"];
    cellView.delegate               = self;
    cellView.filePath               = nil;
    cellView.infoTextField.hidden   = NO;
    cellView.sizeLabel.hidden       = YES;
    cellView.openButton.hidden      = YES;
    cellView.openFolderButton.hidden = YES;
    cellView.openButton.title = NSLocalizedString(@"Open", nil);
    cellView.openFolderButton.title = NSLocalizedString(@"Show in folder", nil);
    
    [self configureCell:cellView withDownload:download];
    return cellView;
}

- (BOOL)selectionShouldChangeInTableView:(NSTableView *)tableView {
    return NO;
}

#pragma mark -
#pragma mark ASCDownloadCellView Delegate

- (void)onCancelButton:(ASCDownloadCellView *)cell {
    ASCDownloadController *controller = [ASCDownloadController sharedInstance];
    id download = [controller downloadWithId:cell.uuid];
    if (download) {
        if (![download[@"canceled"] boolValue]) {
            download[@"canceled"] = @(YES);
            if (![download[@"complete"] boolValue]) {
                [controller cancelDownload:cell.uuid];
            }
        }
    }
}

#pragma mark -
#pragma mark ASCDownloadController Delegate

- (void)downloadController:(ASCDownloadController *)controler didAddDownload:(id)download {
    NSString *uuid = download[@"idx"];
    if (!uuid || [_rowUUIDs containsObject:uuid]) return;

    [_rowUUIDs addObject:uuid];
    NSInteger newRow = (NSInteger)_rowUUIDs.count - 1;

    [self updatePopoverSize];

    [self.tableView beginUpdates];
    [self.tableView insertRowsAtIndexes:[NSIndexSet indexSetWithIndex:newRow] withAnimation:NSTableViewAnimationEffectNone];
    [self.tableView endUpdates];

    [self updateClearButton];
}

- (void)downloadController:(ASCDownloadController *)controler didRemovedDownload:(id)download {
    NSString *uuid = download[@"idx"];
    if (!uuid) return;
    
    NSInteger row = (NSInteger)[_rowUUIDs indexOfObject:uuid];
    if (row != (NSInteger)NSNotFound) {
        [self.tableView beginUpdates];
        [self.tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndex:row] withAnimation:NSTableViewAnimationEffectNone];
        [self.tableView endUpdates];
    }
    
    [_rowUUIDs removeObject:uuid];
    
    if (_rowUUIDs.count == 0) {
        NSWindow *popoverWindow = self.view.window;
        if (popoverWindow) {
            [[popoverWindow parentWindow] removeChildWindow:popoverWindow];
            [popoverWindow orderOut:nil];
        }
    } else {
        [self updatePopoverSize];
        [self updateClearButton];
    }
}

- (void)downloadController:(ASCDownloadController *)controler didUpdatedDownload:(id)download {
    NSString *uuid = download[@"idx"];
    if (!uuid) return;
    
    NSInteger row = (NSInteger)[_rowUUIDs indexOfObject:uuid];
    if (row != (NSInteger)NSNotFound) {
        ASCDownloadCellView *cell = [self.tableView viewAtColumn:0 row:row makeIfNecessary:NO];
        if (cell) [self configureCell:cell withDownload:download];
    }
    
    [self updateClearButton];
}

@end

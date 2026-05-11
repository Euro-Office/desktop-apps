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
//  ASCDownloadCellView.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/29/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCDownloadCellView.h"
#import "ASCHeaderButton.h"
#import "ASCThemesController.h"
#import "ASCConstants.h"

@interface ASCDownloadCellView()
@property (nonatomic) NSTrackingArea *hoverTrackingArea;
@end

@implementation ASCDownloadCellView

- (void)setBackgroundStyle:(NSBackgroundStyle)backgroundStyle {
    [super setBackgroundStyle: NSBackgroundStyleLight];
}

- (void)awakeFromNib {
    [super awakeFromNib];
    [self applyThemeColors];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onThemeChanged:)
                                                 name:ASCEventNameChangedUITheme
                                               object:nil];
}

- (void)applyThemeColors {
    NSString *currentTheme = [ASCThemesController currentThemeId];
    
    NSColor *textColor = [ASCThemesController color:downloadLabelTextColor forTheme:currentTheme];
    if (textColor && self.textField) {
        self.textField.textColor = textColor;
    }
    
    NSColor *infoColor = [ASCThemesController color:downloadLabelTextInfoColor forTheme:currentTheme];
    if (infoColor && self.infoTextField) {
        self.infoTextField.textColor = infoColor;
    }
    if (infoColor && self.sizeLabel) {
        self.sizeLabel.textColor = infoColor;
    }
    
    NSColor *buttonTextColor = [ASCThemesController color:downloadGhostButtonTextColor forTheme:currentTheme];
    if (buttonTextColor) {
        if (self.cancelButton) {
            if (@available(macOS 10.14, *)) {
                self.cancelButton.contentTintColor = buttonTextColor;
            }
        }
        if (self.openButton) {
            if (@available(macOS 10.14, *)) {
                self.openButton.contentTintColor = buttonTextColor;
            }
        }
        if (self.openFolderButton) {
            if (@available(macOS 10.14, *)) {
                self.openFolderButton.contentTintColor = buttonTextColor;
            }
        }
    }
}

- (void)onThemeChanged:(NSNotification *)notification {
    [self applyThemeColors];
}

- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    
    if (self.hoverTrackingArea) {
        [self removeTrackingArea:self.hoverTrackingArea];
    }

    self.hoverTrackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                          options:NSTrackingMouseEnteredAndExited |
                                                                  NSTrackingActiveInKeyWindow |
                                                                  NSTrackingInVisibleRect
                                                            owner:self
                                                         userInfo:nil];

    [self addTrackingArea:self.hoverTrackingArea];
}

- (void)mouseEntered:(NSEvent *)event {
    self.wantsLayer = YES;
    
    NSString *currentTheme = [ASCThemesController currentThemeId];
    NSColor *hoverColor = [ASCThemesController color:downloadItemHoverBackgroundColor forTheme:currentTheme];
    if (hoverColor) {
        self.layer.backgroundColor = hoverColor.CGColor;
    }
    
    if (!_sizeLabel.isHidden) {
        _infoTextField.hidden    = YES;
        _openButton.hidden       = NO;
        _openFolderButton.hidden = NO;
    }
}

- (void)mouseExited:(NSEvent *)event {
    self.layer.backgroundColor = [NSColor clearColor].CGColor;
    
    _infoTextField.hidden    = NO;
    _openButton.hidden       = YES;
    _openFolderButton.hidden = YES;
}

- (IBAction)onCancelButton:(NSButton *)sender {
    if (_delegate && [_delegate respondsToSelector:@selector(onCancelButton:)]) {
        [_delegate onCancelButton:self];
    }
}

- (IBAction)onOpenButton:(NSButton *)sender {
    if (_filePath.length > 0) {
        [[NSWorkspace sharedWorkspace] openFile:_filePath];
    }
}

- (IBAction)onShowInFolderButton:(NSButton *)sender {
    if (_filePath.length > 0) {
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs: @[[NSURL fileURLWithPath:_filePath]]];
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

@end

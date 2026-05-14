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
//  ASCTabTouchBarItem.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 02/01/2019.
//  Copyright (c) 2019 Ascensio System SIA. All rights reserved.
//

#import "ASCTabTouchBarItem.h"
#import "ASCTabTouchBar.h"
#import "NSColor+Extensions.h"

@interface ASCTabTouchBarItem ()
@property (strong) NSImageView *imageView;
@property (strong) NSTextField *textField;
@property (strong) NSProgressIndicator *indicator;
@end

@implementation ASCTabTouchBarItem

- (void)setTabInfo:(ASCTabTouchBar *)tabInfo {
    _tabInfo = tabInfo;
    [self updateInfo];
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    
    if (self) {
        _textField = [[NSTextField alloc] initWithFrame:NSZeroRect];
        _imageView = [[NSImageView alloc] initWithFrame:NSZeroRect];
        _indicator = [[NSProgressIndicator alloc] initWithFrame:NSZeroRect];
        
        _textField.font = [NSFont systemFontOfSize: 14];
        _textField.textColor = [NSColor alternateSelectedControlTextColor];
        
        _textField.alignment = [self userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionLeftToRight ?
                                    NSTextAlignmentLeft : NSTextAlignmentRight;
        _textField.lineBreakMode = NSLineBreakByTruncatingTail;
        
        [self addSubview:_imageView];
        [self addSubview:_textField];
        
        self.layer.backgroundColor = [[NSColor lightGrayColor] CGColor];
        self.layer.cornerRadius = 5;
        
        [self updateLayout];
    }
    
    return self;
}

- (void)updateLayout {
    if (_imageView && _textField) {
        _imageView.translatesAutoresizingMaskIntoConstraints = NO;
        _textField.translatesAutoresizingMaskIntoConstraints = NO;
        
        NSTextField *targetTextField = self.textField;
        NSImageView *targetImageView = self.imageView;
        
        NSDictionary *viewBindings = NSDictionaryOfVariableBindings(targetImageView, targetTextField);
        NSString *formatString = @"H:|-4-[targetImageView]-0-[targetTextField]-2-|";
        NSArray *hConstraints = [NSLayoutConstraint constraintsWithVisualFormat:formatString
                                                                        options:0
                                                                        metrics:nil
                                                                          views:viewBindings];
        
        formatString = @"V:|-0-[targetImageView]-0-|";
        NSArray *vConstraints = [NSLayoutConstraint constraintsWithVisualFormat:formatString
                                                                        options:0
                                                                        metrics:nil
                                                                          views:viewBindings];
        
        NSLayoutConstraint *alignConstraint = [NSLayoutConstraint constraintWithItem:self.imageView
                                                                           attribute:NSLayoutAttributeCenterY
                                                                           relatedBy:NSLayoutRelationEqual
                                                                              toItem:self.textField
                                                                           attribute:NSLayoutAttributeCenterY
                                                                          multiplier:1
                                                                            constant:0];
        NSMutableArray *constraints = [NSMutableArray arrayWithArray:hConstraints];
        [constraints addObjectsFromArray:vConstraints];
        [constraints addObject:alignConstraint];
        
        [NSLayoutConstraint activateConstraints:constraints];
        
        [self updateInfo];
    }
}

- (void)updateInfo {
    if (_tabInfo && _imageView && _textField) {       
        _textField.textColor = [NSColor alternateSelectedControlTextColor];
        
        switch (_tabInfo.type) {
            case ASCTabTouchBarTypeDocument:
                self.layer.backgroundColor = [[NSColor brendDocumentEditor] CGColor];
                // self.layer.backgroundColor = [[NSColor themedDocumentEditor] CGColor];
                [_imageView setImage:[NSImage imageNamed:@"touchbar-tab-document"]];
                break;
            case ASCTabTouchBarTypeSpreadsheet:
                self.layer.backgroundColor = [[NSColor brendSpreadsheetEditor] CGColor];
                [_imageView setImage:[NSImage imageNamed:@"touchbar-tab-spreadsheet"]];
                break;
            case ASCTabTouchBarTypePresentation:
                self.layer.backgroundColor = [[NSColor brendPresentationEditor] CGColor];
                [_imageView setImage:[NSImage imageNamed:@"touchbar-tab-presentation"]];
                break;
            case ASCTabTouchBarTypePdf:
                self.layer.backgroundColor = [[NSColor brandPdfEditor] CGColor];
                [_imageView setImage:[NSImage imageNamed:@"touchbar-tab-document"]];
                break;
            default:
                self.layer.backgroundColor = [[NSColor whiteColor] CGColor];
                [_imageView setImage:[NSImage imageNamed:NSImageNameTouchBarOpenInBrowserTemplate]];
                _textField.textColor = [NSColor blackColor];
                break;
        }
        [_textField setStringValue:_tabInfo.title];
    }
}

@end

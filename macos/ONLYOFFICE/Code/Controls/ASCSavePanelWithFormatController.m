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
//  ASCSavePanelWithFormat.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 12/11/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCSavePanelWithFormatController.h"

@interface ASCSavePanelWithFormatController()
@property (nonatomic) NSPopUpButton * popupFormats;
@end

@implementation ASCSavePanelWithFormatController

- (NSSavePanel *)savePanel {
    if (nil == _savePanel) {
        _savePanel = [NSSavePanel savePanel];
        [self initAccessoryView];
    }
    
    return _savePanel;
}

- (void)initAccessoryView {
    NSView  *accessoryView = [[NSView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 400, 64.0)];
    
    NSTextField *label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 20, 120, 22)];
    [label setEditable:NO];
    [label setStringValue:NSLocalizedString(@"File Format:", nil)];
    [label setBordered:NO];
    [label setBezeled:NO];
    [label setDrawsBackground:NO];
    [label setAlignment:NSRightTextAlignment];
    
    _popupFormats = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(125.0, 22, 255, 22.0) pullsDown:NO];
    [_popupFormats setTarget:self];
    [_popupFormats setAction:@selector(selectFormat:)];
    
    [accessoryView addSubview:label];
    [accessoryView addSubview:_popupFormats];
    
    [[self savePanel] setAccessoryView:accessoryView];
}

- (void)selectFormat:(id)sender {
    NSPopUpButton * button = (NSPopUpButton *)sender;
    NSInteger selectedItemIndex = [button indexOfSelectedItem];
    
    _filterType = [self.filters[selectedItemIndex][@"type"] intValue];
    [[self savePanel] setAllowedFileTypes:@[self.filters[selectedItemIndex][@"extension"]]];
}

- (void)setFilters:(NSArray *)filters {
    _filters = filters;
    
    for (NSDictionary * filter in filters) {
        [_popupFormats addItemWithTitle:[NSString stringWithFormat:@"%@ (.%@)", filter[@"description"], filter[@"extension"]]];
    }
    
    if (_filters && [_filters count] > 0) {
        [[self savePanel] setAllowedFileTypes:@[[_filters firstObject][@"extension"]]];
    }
}

- (void)setFilterType:(NSInteger)filterType {
    if ([_filters count] < 1) {
        return;
    }
    
    _filterType = filterType;

    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"self.type == %@", @(filterType)];
    NSUInteger index = [_filters indexOfObjectPassingTest:^BOOL(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        return [predicate evaluateWithObject:obj];
    }];
    
    if (NSNotFound == index) {
        if ( _original ) {
            NSString * postfix = _original[@"typeInfo"][@"extension"];
            if ( postfix ) {
                predicate = [NSPredicate predicateWithFormat:@"self.extension == %@", postfix];
                index = [_filters indexOfObjectPassingTest:^BOOL(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                    return [predicate evaluateWithObject:obj];
                }];
            }
        }
    }

    if ( NSNotFound == index ) {
        NSInteger selectIndex = MAX([_popupFormats indexOfSelectedItem], 0);
        _filterType = [_filters[selectIndex][@"type"] intValue];
    } else {
        [_popupFormats selectItemAtIndex:index];
        [[self savePanel] setAllowedFileTypes:@[_filters[index][@"extension"]]];
    }
}

- (void)setOriginal:(NSDictionary *)original {
    _original = original;
    [self setFilterType:[original[@"type"] intValue]];
}

@end

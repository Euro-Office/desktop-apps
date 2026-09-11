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

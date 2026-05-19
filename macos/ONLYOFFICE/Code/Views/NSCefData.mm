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
//  NSCefData.m
//
//  Copyright (c) 2025 Ascensio System. All rights reserved.
//

#import "NSCefData.h"

@interface NSCefData () {
    CefViewWrapperType _viewType;
    BOOL _isClosed;
    BOOL _isReadonly;
    BOOL _isChanged;
    BOOL _hasChanges;
    NSString *_features;
    NSString *_strReadonly;
}
@end


@implementation NSCefData

- (instancetype)init {
    self = [super init];
    return self;
}

-(instancetype)initWith:(NSString *)title viewType:(CefViewWrapperType)type {
    self = [super init];
    self.title = title;
    _isLocal = NO;
    _viewType = type;
    _contentType = AscEditorType::etUndefined;
    return self;
}

-(instancetype)initWith:(NSString *)title contentType:(AscEditorType)type {
    self = [super init];
    self.title = title;
    _contentType = type;
    switch (type) {
    case AscEditorType::etDocument:
    case AscEditorType::etSpreadsheet:
    case AscEditorType::etPresentation:
    case AscEditorType::etPdf:
    case AscEditorType::etDraw:
        _viewType = cvwtEditor;
        break;
    default:
        _viewType = cvwtSimple;
        break;
    }
    return self;
}

-(NSString *)title:(BOOL)orig {
    if (orig) {
        return self.title;
    }
    
    NSMutableString *output = [NSMutableString stringWithString:_title ?: @""];
    if (_hasChanges) {
        [output insertString:@"*" atIndex:0];
    }
    
    if (_isReadonly) {
        [output appendString:_strReadonly];
    }
    
    return [output copy];
}

-(NSString *)features {
    return _features;
}

-(CefViewWrapperType)viewType {
    return _viewType;
}

-(BOOL)isViewType:(CefViewWrapperType)viewType {
    return _viewType == viewType;
}

-(BOOL)hasFeature:(NSString *)feature {
    if (!feature || !_features)
        return NO;
    
    NSRange range = [_features rangeOfString:feature];
    return range.location != NSNotFound;
}

-(BOOL)hasFrame {
    // TODO: add implementation
    return NO;
}

-(BOOL)modified {
    return _isChanged;
}

-(BOOL)hasChanges {
    return _hasChanges;
}

-(BOOL)closed {
    return _isClosed;
}

-(void)setFeatures:(NSString *)features {
    // TODO: add implementation
}

-(void)setChanged:(BOOL)changed {
    _hasChanges = changed;
    if (changed && !_isChanged)
        _isChanged = YES;
}

-(void)close {
    _isClosed = YES;
}

-(void)reuse {
    _isClosed = NO;
}

@end

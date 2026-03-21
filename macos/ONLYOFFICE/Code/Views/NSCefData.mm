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
//  NSCefData.m
//
//  Copyright (c) 2025 Ascensio System. All rights reserved.
//

#import "NSCefData.h"
#import "ASCProviders.h"

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
    return [[ASCProviders sharedInstance] editorsHasFrameWithUrl:_url cloud:_cloudName];
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

-(void)setFeatures:(NSString *)fs {
    if (!fs || fs.length == 0) {
        return;
    }
    
    NSError *error = nil;
    NSData *jsonData = [fs dataUsingEncoding:NSUTF8StringEncoding];
    id parsedObj = [NSJSONSerialization JSONObjectWithData:jsonData
                                                   options:0
                                                     error:&error];
    
    if (!error && [parsedObj isKindOfClass:[NSDictionary class]]) {
        NSDictionary *newDict = (NSDictionary *)parsedObj;
        
        if (!_features || _features.length == 0) {
            NSData *compactData = [NSJSONSerialization dataWithJSONObject:newDict
                                                                  options:0
                                                                    error:nil];
            _features = [[NSString alloc] initWithData:compactData encoding:NSUTF8StringEncoding];
            // NSLog(@"JSON: %@", _features);
            
        } else {
            NSData *existingData = [_features dataUsingEncoding:NSUTF8StringEncoding];
            id existingObj = [NSJSONSerialization JSONObjectWithData:existingData
                                                             options:0
                                                               error:&error];
            
            if (!error && [existingObj isKindOfClass:[NSDictionary class]]) {
                NSMutableDictionary *mergedDict = [NSMutableDictionary dictionaryWithDictionary:(NSDictionary *)existingObj];
                [mergedDict addEntriesFromDictionary:newDict];
                NSData *resultData = [NSJSONSerialization dataWithJSONObject:mergedDict
                                                                     options:0
                                                                       error:nil];
                if (resultData) {
                    _features = [[NSString alloc] initWithData:resultData encoding:NSUTF8StringEncoding];
                    // NSLog(@"JSON: %@", _features);
                }
            } else {
                NSLog(@"JSON has error: %@", error.localizedDescription);
            }
        }
    } else {
        NSLog(@"JSON has error: %@", error.localizedDescription);
    }
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

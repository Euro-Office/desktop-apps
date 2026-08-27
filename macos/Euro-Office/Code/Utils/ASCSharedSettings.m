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
//  ASCSharedSettings.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 12/15/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCSharedSettings.h"

@interface ASCSharedSettings()
@property (nonatomic) NSMutableDictionary * settings;
@end

@implementation ASCSharedSettings

+ (instancetype)sharedInstance {
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
        _settings = [NSMutableDictionary dictionary];
    }
    
    return self;
}

- (void)setSetting:(id)setting forKey:(id <NSCopying>)aKey {
    if (setting) {
        [_settings setObject:setting forKey:aKey];
    } else {
        [_settings removeObjectForKey:aKey];
    }
}

- (id)settingByKey:(id)key {
    return [_settings objectForKey:key];
}

@end

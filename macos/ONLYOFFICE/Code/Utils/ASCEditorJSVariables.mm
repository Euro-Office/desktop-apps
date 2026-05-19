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
//  ASCEditorJSVariables.m
//  ONLYOFFICE
//
//  Created by Maxim.Kadushkin on 28/07/2022.
//  Copyright © 2022 Ascensio System SIA. All rights reserved.
//


#import "ASCEditorJSVariables.h"
#import "ASCApplicationManager.h"
#import "NSDictionary+Extensions.h"
#import "NSString+Extensions.h"
#import "ASCLinguist.h"
#import "ASCConstants.h"


@interface ASCEditorJSVariables()
@property (nonatomic) NSMutableDictionary * jsVariables;
@property (nonatomic) NSMutableDictionary * urlParams;
@end

@implementation ASCEditorJSVariables

+ (instancetype)instance {
    static id instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });

    return instance;
}

- (id)init {
    self = [super init];
    if (self) {
        _jsVariables = [NSMutableDictionary dictionary];
        _urlParams = [NSMutableDictionary dictionary];

        NSString * url = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserWebappsHelpUrl];
        if ( url == nil || url.length == 0 ) {
            url = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"ASCWebappsHelpUrl"];
        }

        if (url && [url length]) {
            [_jsVariables setValue:url forKey:@"helpUrl"];
            NSLog(@"set web-apps help url %@", url);
        }


        if (url && [url length]) {
            [_jsVariables setValue:url forKey:@"helpUrl"];
            NSLog(@"set web-apps help url %@", url);
        }

        [self setParameter:@"lang" withString:[ASCLinguist appLanguageCode]];

        NSMutableDictionary * keybLangs = [NSMutableDictionary dictionary];
        [keybLangs setObject:[ASCLinguist keyboardLanguages] forKey:@"langs"];
        [_jsVariables setObject:keybLangs forKey:@"keyboard"];
        [_jsVariables setValue:[ASCLinguist appLanguageCode] forKey:@"lang"];
    }

    return self;
}

- (void)apply {
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    appManager->SetRendererProcessVariable([[_jsVariables jsonString] stdwstring]);
}

- (void)setVariable: (NSString*)name withString:(NSString *)value {
    [_jsVariables setValue:value forKey:name];
}

- (void)setVariable: (NSString*)name withBool:(BOOL)value {
    [_jsVariables setValue:@(value) forKey:name];
}

- (void)setVariable: (NSString*)name withObject:(NSDictionary *)object {
    [_jsVariables setObject:object forKey:name];
}

- (void)setParameter:(NSString*)name withString:(NSString *)value {
    [_urlParams setValue:value forKey:name];
}

- (void)removeParameter:(NSString*)name {
    [_urlParams removeObjectForKey:name];
}

- (void)applyParameters {
    NSMutableString * str = [[NSMutableString alloc] init];

    for(id key in _urlParams)
        [str appendFormat:@"%@=%@&", key, [_urlParams objectForKey:key]];

    if ( [str hasSuffix:@"&"] )
        [str deleteCharactersInRange:NSMakeRange([str length] - 1, 1)];

    std::wstring wParams = [str stdwstring];
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    appManager->InitAdditionalEditorParams(wParams);
}

@end

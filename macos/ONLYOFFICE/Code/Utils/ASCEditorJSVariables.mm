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

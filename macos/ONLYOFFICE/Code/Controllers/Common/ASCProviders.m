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
//  ASCProviders.m
//  ONLYOFFICE
//
//  Copyright © 2026 Ascensio System SIA. All rights reserved.
//

#import "ASCProviders.h"

@interface ProviderData : NSObject
@property (nonatomic, copy) NSString *provider;
@property (nonatomic, copy) NSString *editorPage;
@property (nonatomic, assign) BOOL hasFrame;
@property (nonatomic, assign) BOOL useRegex;
@end

@implementation ProviderData
@end

@interface ASCProviders ()
@property (nonatomic, strong) NSMutableArray<ProviderData *> *providers;
@end

@implementation ASCProviders

+ (instancetype)sharedInstance {
    static ASCProviders *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[ASCProviders alloc] init];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _providers = [NSMutableArray array];
    }
    return self;
}

- (void)configureWithJson:(NSString *)jsonString {
    NSData *data = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
    NSError *error = nil;
    id json = [NSJSONSerialization JSONObjectWithData:data
                                              options:0
                                                error:&error];

    if (error || ![json isKindOfClass:[NSArray class]]) {
        return;
    }

    NSArray *array = (NSArray *)json;
    for (NSDictionary *obj in array) {
        ProviderData *pd = [[ProviderData alloc] init];
        pd.provider = [obj[@"provider"] lowercaseString];
        pd.hasFrame = [obj[@"editorFrameSize"] isEqualToString:@"finite"];
        pd.editorPage = [obj[@"editorPage"] copy];

        NSString *regPrefix = @"regex:";
        NSRange range = [pd.editorPage rangeOfString:regPrefix];
        if (range.location != NSNotFound) {
            pd.useRegex = YES;
            pd.editorPage = [pd.editorPage substringFromIndex:range.location + regPrefix.length];
        }

        [self.providers addObject:pd];
    }
}

- (BOOL)editorsHasFrameWithUrl:(NSString *)url cloud:(NSString *)cloud {
    for (ProviderData *pd in self.providers) {
        if (pd.provider.length > 0 && [pd.provider isEqualToString:cloud]) {
            return pd.hasFrame;
        }

        if (pd.editorPage.length > 0 && url) {
            if (pd.useRegex) {
                NSError *error = nil;
                NSRegularExpression *regex =
                [NSRegularExpression regularExpressionWithPattern:pd.editorPage
                                                          options:NSRegularExpressionCaseInsensitive
                                                            error:&error];
                if (!error) {
                    NSUInteger matches =
                    [regex numberOfMatchesInString:url
                                            options:0
                                              range:NSMakeRange(0, url.length)];
                    if (matches > 0)
                        return pd.hasFrame;
                }

            } else {
                if ([url rangeOfString:pd.editorPage].location != NSNotFound)
                    return pd.hasFrame;
            }
        }
    }

    return NO;
}

@end

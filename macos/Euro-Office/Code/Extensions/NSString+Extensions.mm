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
//  NSString+Extensions.mm
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 7/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "NSString+Extensions.h"
/** Need for MD5 & SHA hashes */
#import <CommonCrypto/CommonDigest.h>

@implementation NSString (Extensions)

+ (id)stringWithstdwstring:(const std::wstring&)string
{
    if (string.length() < 1) {
        return @"";
    }
    
    return [[NSString alloc] initWithBytes:(char*)string.data()
                                    length:string.size()* sizeof(wchar_t)
                                  encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)];
}

- (std::wstring)stdwstring
{
    NSStringEncoding encode = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
    NSData* data = [self dataUsingEncoding:encode];
    
    return std::wstring ((wchar_t*)[data bytes], [data length] / sizeof(wchar_t));
}

- (std::string)stdstring
{
    return std::string([self UTF8String]);
}

+ (NSMutableArray*)stringsArray:(const std::vector<std::wstring>&)sources
{
    size_t count = sources.size();
    NSMutableArray* array = [NSMutableArray arrayWithCapacity:count];
    for (size_t i = 0; i < count; ++i) {
        [array addObject:[NSString stringWithstdwstring:sources[i]]];
    }
    return array;
}

- (NSString *)stringByAppendingUrlQuery:(NSString *)query {
    if (![query length]) {
        return self;
    }
    
    return [NSString stringWithFormat:@"%@%@%@", self, ([self rangeOfString:@"?"].length > 0) ? @"&" : @"?", query];
}

- (NSString *)md5 {
    NSUInteger length = CC_MD5_DIGEST_LENGTH;
    
    const char *cStr = [self UTF8String];
    unsigned char result[length];
    
    CC_MD5(cStr, (CC_LONG)strlen(cStr), result);
    
    NSMutableString *output = [NSMutableString stringWithCapacity:length * 2];
    
    for(int i = 0; i < length; i++)
        [output appendFormat:@"%02x", result[i]];
    
    return output;
}

- (NSDictionary *)dictionary {
    NSError * error;
    NSData *data = [self dataUsingEncoding:NSUTF8StringEncoding];
    id json = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];

    if (error) {
        NSLog(@"NSJSONSerialization Error: %@", error);
    }

    return json;
}

- (NSString *)removeUrlQuery:(NSArray<NSString *> *)params {
    if (NSURLComponents *components = [NSURLComponents componentsWithString:self]) {
        NSMutableArray<NSURLQueryItem *> *newQueryItems = [[components queryItems] mutableCopy];

        for (NSString *param in params) {
            NSUInteger index = [newQueryItems indexOfObjectPassingTest:^BOOL(NSURLQueryItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                return [param isEqualToString:obj.name];
            }];

            if (NSNotFound != index) {
                [newQueryItems removeObjectAtIndex:index];
            }
        }

        if (newQueryItems.count < 1) {
            components.query = nil;
        } else {
            components.queryItems = newQueryItems;
        }
        
        components.fragment = nil;

        return [components string];
    }
    return self;
}

- (NSString *)virtualUrl {
    if (NSURLComponents *components = [NSURLComponents componentsWithString:self]) {
        components.query = nil;
        components.fragment = nil;
        return [components string];
    }

    return self;
}

@end

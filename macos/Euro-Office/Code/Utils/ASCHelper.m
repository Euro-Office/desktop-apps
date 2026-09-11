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
//  ASCHelper.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/8/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCHelper.h"
#import "ASCExternalController.h"

static NSMutableDictionary * localSettings;

@implementation ASCHelper

+ (NSString *)applicationDataPath {
    NSError * error;
    NSString * path = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) firstObject];
#ifndef MAS
    path = [path stringByAppendingPathComponent:[[NSBundle mainBundle] bundleIdentifier]];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:path]) {
        [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:NO attributes:nil error:&error];
        
        if (error) {
            NSLog(@"Error creating application path: %@", [error localizedDescription]);
        }
    }
#endif
    
    return path;
}

+ (NSString *)recoveryDataPath {
    NSError * error;
    NSString * applicationDataPath = [self applicationDataPath];
    
    NSString * recoveryPath = [applicationDataPath stringByAppendingPathComponent:@"recovery"];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:recoveryPath]) {
        [[NSFileManager defaultManager] createDirectoryAtPath:recoveryPath withIntermediateDirectories:NO attributes:nil error:&error];
        
        if (error) {
            NSLog(@"Error copying application path: %@", [error localizedDescription]);
        }
    }

    
    return recoveryPath;
}

+ (NSString *)licensePath {
    NSError * error;
    NSString * applicationDataPath = [self applicationDataPath];
    NSString * licenseDirectory = [applicationDataPath stringByAppendingPathComponent:@"license"];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:licenseDirectory]) {
        [[NSFileManager defaultManager] createDirectoryAtPath:licenseDirectory withIntermediateDirectories:NO attributes:nil error:&error];
        
        if (error) {
            NSLog(@"Error copying application path: %@", [error localizedDescription]);
        }
    }
    
    return licenseDirectory;
}

+ (void)createCloudPath {
    NSError * error;
    NSString * applicationDataPath = [self applicationDataPath];
        
    NSString * webDataPath = [applicationDataPath stringByAppendingPathComponent:@"webdata"];
    NSString * cloudPath   = [webDataPath stringByAppendingPathComponent:@"cloud"];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:webDataPath]) {
        if (![[NSFileManager defaultManager] createDirectoryAtPath:webDataPath withIntermediateDirectories:NO attributes:nil error:&error]) {
            NSLog(@"Error create webdata path: %@", [error localizedDescription]);
        }
    }
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:cloudPath]) {
        if (![[NSFileManager defaultManager] createDirectoryAtPath:cloudPath withIntermediateDirectories:NO attributes:nil error:&error]) {
            NSLog(@"Error create cloud path: %@", [error localizedDescription]);
        }
    }
}

+ (NSMutableDictionary *)localSettings {
    if (!localSettings)
        localSettings = [NSMutableDictionary dictionary];
    
    return localSettings;
}

+ (NSString *)appName {
    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onApplicationName)]) {
        return [externalDelegate onApplicationName];
    } else {
        CFBundleRef localInfoBundle = CFBundleGetMainBundle();
        NSDictionary * localInfoDict = (NSDictionary *)CFBundleGetLocalInfoDictionary(localInfoBundle);

        if (localInfoDict) {
            NSString * productName = [localInfoDict objectForKey:@"CFBundleName"];

            if (productName && productName.length > 0) {
                return productName;
            }
        }

        return [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString *)kCFBundleNameKey];
    }
}

+ (NSString *)appNameShort {
    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onApplicationNameShort)]) {
        return [externalDelegate onApplicationNameShort];
    } else {
        CFBundleRef localInfoBundle = CFBundleGetMainBundle();
        NSDictionary * localInfoDict = (NSDictionary *)CFBundleGetLocalInfoDictionary(localInfoBundle);

        if (localInfoDict) {
            NSString * productName = [localInfoDict objectForKey:@"CFBundleName"];

            if (productName && productName.length > 0) {
                return productName;
            }
        }

        return [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString *)kCFBundleNameKey];
    }
}

@end

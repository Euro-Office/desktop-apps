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
//  ASCDownloadController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/28/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCDownloadController.h"
#import "NSString+Extensions.h"
#import "mac_application.h"
#import <objc/runtime.h>

static NSString * const kASCDownloadControllerMulticastDelegateKey = @"ASCDownloadControllersMulticastDelegate";

@interface ASCDownloadController ()
@property (nonatomic, assign) id <ASCDownloadControllerDelegate> delegate;
@end

@implementation ASCDownloadController

- (id)init {
    self = [super init];
    
    if (self) {
        _downloads = [NSMutableArray array];
    }
    
    return self;
}

+ (instancetype)sharedInstance
{
    static id sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    
    return sharedInstance;
}

- (id)downloadWithId:(NSString *)idx {
    NSPredicate *bPredicate = [NSPredicate predicateWithFormat:@"SELF.idx == %@", idx];
    NSArray * filteredArray = [_downloads filteredArrayUsingPredicate:bPredicate];
    
    return filteredArray.firstObject;
}

- (ASCMulticastDelegate *)multicastDelegate {
        id multicastDelegate = objc_getAssociatedObject(self, (__bridge const void *)(kASCDownloadControllerMulticastDelegateKey));
    
        if (multicastDelegate == nil) {
            // if not, create one
            multicastDelegate = [[ASCMulticastDelegate alloc] init];
            objc_setAssociatedObject(self, (__bridge const void *)(kASCDownloadControllerMulticastDelegateKey), multicastDelegate, OBJC_ASSOCIATION_RETAIN);
            
            // and set it as the delegate
            self.delegate = multicastDelegate;
        }
        
        return multicastDelegate;
}

- (void)addDownload:(NSString *)idx fileName:(NSString *)fileName {
    NSMutableDictionary * download = [NSMutableDictionary dictionaryWithDictionary:@{
                                                                                     @"idx"      : idx,
                                                                                     @"name"     : fileName,
                                                                                     @"url"      : @"",
                                                                                     @"filePath" : @"",
                                                                                     @"percent"  : @0,
                                                                                     @"complete" : @(NO),
                                                                                     @"canceled" : @(NO),
                                                                                     @"speed"    : @0
                                                                                     }];

    [_downloads addObject:download];
    
    if (_delegate && [_delegate respondsToSelector:@selector(downloadController:didAddDownload:)]) {
        [_delegate downloadController:self didAddDownload:download];
    }
}

- (void)removeDownload:(NSString *)idx {
    id download = [self downloadWithId:idx];
    
    if (download) {
        NSString * filePath = download[@"filePath"];
        BOOL isCanceled     = [download[@"canceled"] boolValue];

        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        if (isCanceled && filePath && [[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
            [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];
        }
        
        if (appManager) {
            appManager->CancelDownload([download[@"idx"] intValue]);
        }
        
        [_downloads removeObject:download];
        
        if (_delegate && [_delegate respondsToSelector:@selector(downloadController:didRemovedDownload:)]) {
            [_delegate downloadController:self didRemovedDownload:download];
        }
    }
}

- (void)updateDownload:(NSString *)idx data:(NSValue *)data {
    NSEditorApi::CAscDownloadFileInfo * pDownloadFileInfo = (NSEditorApi::CAscDownloadFileInfo *)[data pointerValue];
    id download = [self downloadWithId:idx];
    
    if (download && pDownloadFileInfo) {
        NSString * fileName = [[NSString stringWithstdwstring:pDownloadFileInfo->get_FilePath()] lastPathComponent];
        [download setObject:[NSNumber numberWithInt:pDownloadFileInfo->get_Percent()] forKey:@"percent"];
        [download setObject:(fileName && fileName.length > 0) ? fileName : NSLocalizedString(@"Preparing...", nil) forKey:@"name"];
        [download setObject:[NSString stringWithstdwstring:pDownloadFileInfo->get_Url()] forKey:@"url"];
        [download setObject:[NSString stringWithstdwstring:pDownloadFileInfo->get_FilePath()] forKey:@"filePath"];
        [download setObject:@(pDownloadFileInfo->get_IsCanceled()) forKey:@"canceled"];
        
        if (_delegate && [_delegate respondsToSelector:@selector(downloadController:didUpdatedDownload:)]) {
            [_delegate downloadController:self didUpdatedDownload:download];
        }
    }
}

@end

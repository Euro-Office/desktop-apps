/*
 * (c) Copyright Ascensio System SIA 2010-2021
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
//  ASCCertificateQLPreview.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 10.02.2021.
//  Copyright © 2021 Ascensio System SIA. All rights reserved.
//

#import "ASCCertificateQLPreview.h"

@interface ASCCertificateQLPreview()
@property (nonatomic) NSURL * originalUrl;
@end

@implementation ASCCertificateQLPreview

- (instancetype)init:(NSURL *)url {
    self = [super init];
    if (self) {
        _fileUrl = url;
    }
    return self;
}

- (instancetype)init:(NSString *)path rename:(BOOL)rename {
    self = [super init];
    if (self) {
        if (rename) {
            NSString * linkPath = [path stringByAppendingString:@".cer"];
            if ([[NSFileManager defaultManager] linkItemAtPath:path toPath:linkPath error:nil]) {
                _originalUrl = [NSURL fileURLWithPath:path];
                _fileUrl = [NSURL fileURLWithPath:linkPath];
            }
        } else {
            _fileUrl = [NSURL fileURLWithPath:path];
        }
    }
    return self;
}

- (NSURL *)previewItemURL {
    return _fileUrl;
}

- (NSString *)previewItemTitle {
    return NSLocalizedString(@"Signature Details", nil);
}

- (void)cleanup {
    [[NSFileManager defaultManager] removeItemAtURL:_fileUrl error:nil];
    if (_originalUrl) {
        [[NSFileManager defaultManager] removeItemAtURL:_originalUrl error:nil];
    }
}

@end

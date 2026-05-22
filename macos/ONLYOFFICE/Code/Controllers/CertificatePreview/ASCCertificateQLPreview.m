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

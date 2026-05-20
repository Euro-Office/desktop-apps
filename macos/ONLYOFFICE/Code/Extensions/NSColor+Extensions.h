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
//  NSColor+Extensions.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 11/04/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define kColorRGBA(r, g, b, a)      [NSColor colorWithRed:(r)/255.f green:(g)/255.f blue:(b)/255.f alpha:(a)]
#define kColorRGB(r, g, b)          [NSColor colorWithRed:(r)/255.f green:(g)/255.f blue:(b)/255.f alpha:1.f]
#define UIColorFromRGB(rgbValue)    [NSColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0 \
                                                    green:((float)((rgbValue & 0x00FF00) >>  8))/255.0 \
                                                     blue:((float)((rgbValue & 0x0000FF) >>  0))/255.0 \
                                                    alpha:1.0]
#define UIColorFromRGBA(rgbValue, a) [NSColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0 \
                                                     green:((float)((rgbValue & 0x00FF00) >>  8))/255.0 \
                                                      blue:((float)((rgbValue & 0x0000FF) >>  0))/255.0 \
                                                     alpha:a]

@interface NSColor (OnlyOffice)
+ (NSColor *) brendDocumentEditor;
+ (NSColor *) brendSpreadsheetEditor;
+ (NSColor *) brendPresentationEditor;
+ (NSColor *) brandPdfEditor;
+ (NSColor *) brandDrawEditor;
@end

@interface NSColor (Extensions)
- (BOOL) isLight;
@end

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

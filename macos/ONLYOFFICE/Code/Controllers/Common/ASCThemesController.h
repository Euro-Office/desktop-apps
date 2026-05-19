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
//  ASCThemesController.h
//  ONLYOFFICE
//
//  Created by Maxim.Kadushkin on 23/08/2022.
//  Copyright © 2022 Ascensio System SIA. All rights reserved.
//

#ifndef ASCThemesController_h
#define ASCThemesController_h

#import <Foundation/Foundation.h>
#import <AppKit/NSColor.h>

static NSString * const btnPortalActiveBackgroundColor  = @"portal-button-background-active-color";
static NSString * const tabWordActiveBackgroundColor    = @"tab-word-background-active-color";
static NSString * const tabCellActiveBackgroundColor    = @"tab-cell-background-active-color";
static NSString * const tabSlideActiveBackgroundColor   = @"tab-slide-background-active-color";
static NSString * const tabPdfActiveBackgroundColor     = @"tab-pdf-background-active-color";
static NSString * const tabDrawActiveBackgroundColor    = @"tab-draw-background-active-color";
static NSString * const tabActiveTextColor              = @"tab-editor-text-active-color";
static NSString * const windowBackgroundColor           = @"window-background-color";

@interface ASCThemesController : NSObject

+ (instancetype)sharedInstance;

+ (NSString*)currentThemeId;
+ (BOOL)isCurrentThemeDark;
+ (NSString*)defaultThemeId:(BOOL)isdark;
+ (NSString*)actualThemeId;
+ (NSColor*)color:(NSString*)name forTheme:(NSString*)theme;
+ (NSColor*)currentThemeColor:(NSString*)name;
+ (BOOL)isSystemDarkMode;
+ (BOOL)isDarkWindowAppearance;

@end

#endif /* ASCThemesController_h */

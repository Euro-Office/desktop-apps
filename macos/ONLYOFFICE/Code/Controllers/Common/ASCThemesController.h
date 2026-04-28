/*
 * (c) Copyright Ascensio System SIA 2010-2022
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
static NSString * const windowBackgroundColor           = @"window-background-color";

@interface ASCThemesController : NSObject

+ (instancetype)sharedInstance;

+ (NSString*)currentThemeId;
+ (BOOL)isCurrentThemeDark;
+ (BOOL)isColorDark:(NSColor*)color;
+ (NSString*)defaultThemeId:(BOOL)isdark;
+ (NSString*)actualThemeId;
+ (NSColor*)color:(NSString*)name forTheme:(NSString*)theme;
+ (NSColor*)currentThemeColor:(NSString*)name;
+ (BOOL)isSystemDarkMode;
+ (BOOL)isDarkWindowAppearance;

+ (NSString*)userThemesPath;
+ (BOOL)validateTheme:(NSDictionary*)jsonObj;
+ (BOOL)containsTheme:(NSString*)themeId;
+ (BOOL)isLocalTheme:(NSString*)themeId;
+ (BOOL)addLocalTheme:(NSDictionary*)jsonObj filePath:(NSString*)filePath;
+ (NSArray*)localThemesToJson;
+ (BOOL)checkDestinationThemeFileExist:(NSString*)srcPath;

@end

#endif /* ASCThemesController_h */

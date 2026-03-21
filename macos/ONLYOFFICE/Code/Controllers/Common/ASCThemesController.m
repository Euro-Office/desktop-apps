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
//  ASCThemesController.m
//  ONLYOFFICE
//
//  Created by Maxim.Kadushkin on 23/08/2022.
//  Copyright © 2022 Ascensio System SIA. All rights reserved.
//

#import "ASCThemesController.h"
#import "ASCConstants.h"
#import "ASCSharedSettings.h"
#import "NSColor+Extensions.h"
#import "NSApplication+Extensions.h"
#import "ASCEditorJSVariables.h"


@implementation ASCThemesController

+ (instancetype)sharedInstance {
    static id sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });

    return sharedInstance;
}

- (id)init {
    self = [super init];

    NSString * uiTheme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];
    if ( !uiTheme ) {
        uiTheme = uiThemeSystem;
        [[NSUserDefaults standardUserDefaults] setObject:uiTheme forKey:ASCUserUITheme];
    }
    
    [[ASCEditorJSVariables instance] setParameter:@"uitheme" withString:uiTheme];
    [[ASCEditorJSVariables instance] applyParameters];
    
    NSString * systemColorScheme = [[self class] isSystemDarkMode] ? @"dark" : @"light";
    [[ASCSharedSettings sharedInstance] setSetting:systemColorScheme forKey:kSettingsColorScheme];

    [[ASCEditorJSVariables instance] setVariable:@"theme" withObject:@{@"id":uiTheme,
                                                                       @"system":systemColorScheme,
                                                                       @"type":[[self class] isCurrentThemeDark] ? @"dark" : @"light"}];

    [NSDistributedNotificationCenter.defaultCenter addObserver:self selector:@selector(onSystemThemeChanged:) name:@"AppleInterfaceThemeChangedNotification" object: nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onUIThemeChanged:)
                                                 name:ASCEventNameChangedUITheme
                                               object:nil];

    return self;
}

- (void)onUIThemeChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * theme = params[@"uitheme"];

        [[ASCEditorJSVariables instance] setVariable:@"theme" withObject:@{@"id":theme,
                                                                         @"type":[[self class] isCurrentThemeDark] ? @"dark" : @"light",
                                                                       @"system":[[self class] isSystemDarkMode] ? @"dark" : @"light"}];
        [[ASCEditorJSVariables instance] apply];
    }
}

- (void)onSystemThemeChanged:(NSNotification *)notification {
    NSLog (@"system theme changed %@", notification);

    [[ASCSharedSettings sharedInstance] setSetting:([ASCThemesController isSystemDarkMode] ? @"dark" : @"light") forKey:kSettingsColorScheme];
    [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameChangedSystemTheme object:nil userInfo:@{@"mode": ([NSApplication isSystemDarkMode] ? @"dark" : @"light")}];
}

+ (NSString*)currentThemeId {
    return [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];
}

+ (BOOL)isCurrentThemeDark {
    NSString * theme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];
    if ([uiThemeSystem isEqualToString:theme]) {
        return [self isSystemDarkMode];
    } else return [uiThemeDark isEqualToString:theme] || [uiThemeContrastDark isEqualToString:theme] || [uiThemeNight isEqualToString:theme];
}

+ (BOOL)isColorDark:(NSColor*)color {
    if (!color) return YES;
    
    NSColor *rgbColor = [color colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
    if (!rgbColor) return YES;
    
    CGFloat r, g, b, a;
    [rgbColor getRed:&r green:&g blue:&b alpha:&a];
    
    CGFloat luma = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    return luma < 0.5;
}

+ (NSString*)defaultThemeId:(BOOL)isdark {
    return isdark ? uiThemeNight : uiThemeWhite;
}

+ (NSString*)actualThemeId {
    NSString * theme = [self currentThemeId];
    if ([uiThemeSystem isEqualToString:theme]) {
        return [self defaultThemeId:[self isCurrentThemeDark]];
    } else return theme;
}

+ (NSColor*)currentThemeColor:(NSString*)name {
    return [self color:name forTheme:[self currentThemeId]];
}

+ (NSColor*)color:(NSString*)name forTheme:(NSString*)theme {
    if ( [theme isEqualToString: uiThemeSystem] )
        theme = [self defaultThemeId:[NSApplication isSystemDarkMode]];

    if ([name isEqualToString:tabActiveTextColor]) {
        if ( [theme isEqualToString:uiThemeGray] ) return UIColorFromRGB(0x444);
        else if ( [theme isEqualToString:uiThemeWhite] ) return UIColorFromRGB(0x444);
        else return NSColor.whiteColor;
    } else if ([name isEqualToString:btnPortalActiveBackgroundColor]) {
        if ( [theme isEqualToString:uiThemeDark] ) return UIColorFromRGB(0x404040);
        else if ( [theme isEqualToString:uiThemeContrastDark] ) return UIColorFromRGB(0x2a2a2a);
        else if ( [theme isEqualToString:uiThemeLight] ) return UIColorFromRGB(0xf7f7f7);
        else if ( [theme isEqualToString:uiThemeClassicLight] ) return UIColorFromRGB(0xf7f7f7);
        else if ( [theme isEqualToString:uiThemeGray] ) return UIColorFromRGB(0xf7f7f7);
        else if ( [theme isEqualToString:uiThemeWhite] ) return UIColorFromRGB(0xf3f3f3);
        else if ( [theme isEqualToString:uiThemeNight] ) return UIColorFromRGB(0x222222);
        else {
            if ( @available(macOS 10.13, *) )
                return [NSColor colorNamed:@"tab-portal-activeColor"];
            else return kColorRGBA(255, 255, 255, 1.0);
        }
    } else if ( [name isEqualToString:windowBackgroundColor] ) {
        if ( [theme isEqualToString:uiThemeDark] ) return UIColorFromRGB(0x282828);
        else if ( [theme isEqualToString:uiThemeContrastDark] ) return UIColorFromRGB(0x181818);
        else if ( [theme isEqualToString:uiThemeGray] ) return UIColorFromRGB(0xd9d9d9);
        else if ( [theme isEqualToString:uiThemeNight] ) return UIColorFromRGB(0x383838);
        else if ( [theme isEqualToString:uiThemeWhite] ) return UIColorFromRGB(0xeaeaea);
        else {
            return UIColorFromRGB(0xe4e4e4);
        }
    } else {
        if ( [theme isEqualToString:uiThemeDark] ) return UIColorFromRGB(0x2a2a2a);
        else if ( [theme isEqualToString:uiThemeContrastDark] ) return UIColorFromRGB(0x1e1e1e);
        else if ( [theme isEqualToString:uiThemeNight] ) return UIColorFromRGB(0x222222);
        else if ( [theme isEqualToString:uiThemeGray] ) return UIColorFromRGB(0xf7f7f7);
        else if ( [theme isEqualToString:uiThemeWhite] ) return UIColorFromRGB(0xf3f3f3);
        else {
            if ([name isEqualToString:tabWordActiveBackgroundColor]) {
               return [NSColor brendDocumentEditor];
            } else if ([name isEqualToString:tabCellActiveBackgroundColor]) {
               return [NSColor brendSpreadsheetEditor];
            } else if ([name isEqualToString:tabSlideActiveBackgroundColor]) {
               return [NSColor brendPresentationEditor];
            } else if ([name isEqualToString:tabPdfActiveBackgroundColor]) {
               return [NSColor brandPdfEditor];
            } else if ([name isEqualToString:tabDrawActiveBackgroundColor]) {
               return [NSColor brandDrawEditor];
            }
        }
    }

    return NULL;
}

+ (BOOL)isSystemDarkMode {
    if (@available(macOS 10.14, *)) {
        NSString * appleInterfaceStyle = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];

        if (appleInterfaceStyle && [appleInterfaceStyle length] > 0) {
            return [[appleInterfaceStyle lowercaseString] containsString:@"dark"];
        }
    }

    return false;
}

+ (BOOL)isDarkWindowAppearance {
    BOOL isDarkWindow;
    if (@available(macOS 10.14, *)) {
        isDarkWindow = [ASCThemesController isCurrentThemeDark];
    } else {
        isDarkWindow = [NSApplication isSystemDarkMode];
    }
    return isDarkWindow;
}

@end

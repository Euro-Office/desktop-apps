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


static NSColor * colorFromHexString(NSString *hexString) {
    if (!hexString || hexString.length == 0) return nil;

    NSString *hex = [hexString hasPrefix:@"#"] ? [hexString substringFromIndex:1] : hexString;

    unsigned long long value = 0;
    NSScanner *scanner = [NSScanner scannerWithString:hex];
    if (![scanner scanHexLongLong:&value] || !scanner.isAtEnd) return nil;

    CGFloat r, g, b, a = 1.0;
    NSUInteger len = hex.length;

    if (len == 3) {
        // #rgb to #rrggbb
        unsigned long long rv = (value >> 8) & 0xF;
        unsigned long long gv = (value >> 4) & 0xF;
        unsigned long long bv = value & 0xF;
        r = ((rv << 4) | rv) / 255.0;
        g = ((gv << 4) | gv) / 255.0;
        b = ((bv << 4) | bv) / 255.0;
    } else if (len == 6) {
        r = ((value >> 16) & 0xFF) / 255.0;
        g = ((value >>  8) & 0xFF) / 255.0;
        b = (value & 0xFF) / 255.0;
    } else if (len == 8) {
        // rrggbbaa
        r = ((value >> 24) & 0xFF) / 255.0;
        g = ((value >> 16) & 0xFF) / 255.0;
        b = ((value >>  8) & 0xFF) / 255.0;
        a = (value & 0xFF) / 255.0;
    } else {
        return nil;
    }

    return [NSColor colorWithRed:r green:g blue:b alpha:a];
}

static NSString* jsonKeysForColorName(NSString *name) {
    static NSDictionary<NSString*, NSString*> * map = nil;
    static dispatch_once_t token;
    dispatch_once(&token, ^{
        map = @{
            btnPortalActiveBackgroundColor : @"tool-button-active-background",
            tabWordActiveBackgroundColor   : @"brand-word",
            tabCellActiveBackgroundColor   : @"brand-cell",
            tabSlideActiveBackgroundColor  : @"brand-slide",
            tabPdfActiveBackgroundColor    : @"brand-pdf",
            tabDrawActiveBackgroundColor   : @"brand-draw",
            windowBackgroundColor          : @"window-background",
        };
    });
    return map[name];
}

@interface ASCThemesController ()
@property (nonatomic) NSMutableDictionary<NSString*, NSData*> *builtinThemes;
@property (nonatomic) NSMutableDictionary<NSString*, NSArray*> *localThemes;
@end

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

    _builtinThemes = [NSMutableDictionary dictionary];
    [self loadBuiltinThemes];

    _localThemes = [NSMutableDictionary dictionary];
    [self searchLocalThemes];

    BOOL isDark = [self isDarkThemeId:uiTheme];

    [[ASCEditorJSVariables instance] setVariable:@"theme" withObject:@{@"id":uiTheme,
                                                                       @"system":systemColorScheme,
                                                                       @"type": isDark ?
                                                                       @"dark" : @"light",
                                                                       @"addlocal": @"on"}];

    NSMutableArray *localThemesArr = [NSMutableArray arrayWithCapacity:_localThemes.count];
    for (NSArray *info in _localThemes.allValues) {
        if (info.count < 2) continue;
        NSError *jsonErr = nil;
        id obj = [NSJSONSerialization JSONObjectWithData:info[1] options:0 error:&jsonErr];
        if (!jsonErr && [obj isKindOfClass:[NSDictionary class]]) {
            [localThemesArr addObject:obj];
        }
    }
    if (localThemesArr.count > 0) {
        [[ASCEditorJSVariables instance] setVariable:@"localthemes" withArray:localThemesArr];
        [[ASCEditorJSVariables instance] apply];
    }

    [NSDistributedNotificationCenter.defaultCenter addObserver:self selector:@selector(onSystemThemeChanged:) name:@"AppleInterfaceThemeChangedNotification" object: nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onUIThemeChanged:)
                                                 name:ASCEventNameChangedUITheme
                                               object:nil];

    return self;
}

- (void)loadBuiltinThemes {
    NSArray<NSString*> *themeIds = @[
        @"theme-light", @"theme-classic-light",
        @"theme-dark",  @"theme-contrast-dark",
        @"theme-gray",  @"theme-white", @"theme-night"
    ];
    for (NSString *themeId in themeIds) {
        NSString *path = [[NSBundle mainBundle] pathForResource:themeId
                                                         ofType:@"json"
                                                    inDirectory:@"styles"];
        if (!path) {
            NSLog(@"ASCThemesController: built-in theme not found in bundle: %@", themeId);
            continue;
        }
        NSData *data = [NSData dataWithContentsOfFile:path];
        if (data) {
            _builtinThemes[themeId] = data;
        }
    }
}

- (void)searchLocalThemes {
    NSString *dir = [[self class] userThemesPath];
    NSArray<NSString*> *files = [[NSFileManager defaultManager]
        contentsOfDirectoryAtPath:dir error:nil];

    for (NSString *fileName in files) {
        if (![fileName.pathExtension.lowercaseString isEqualToString:@"json"]) continue;
        NSString *fullPath = [dir stringByAppendingPathComponent:fileName];
        [self addThemeFromFile:fullPath];
    }
}

- (BOOL)addThemeFromFile:(NSString*)path {
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data) return NO;

    NSError *error = nil;
    id obj = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
    if (error || ![obj isKindOfClass:[NSDictionary class]]) return NO;

    NSDictionary *jsonObj = (NSDictionary *)obj;
    if (![[self class] validateTheme:jsonObj]) return NO;

    NSString *themeId = jsonObj[@"id"];
    _localThemes[themeId] = @[path, data];
    return YES;
}

- (BOOL)isDarkThemeId:(NSString*)themeId {
    if ([uiThemeSystem isEqualToString:themeId]) {
        return [[self class] isSystemDarkMode];
    }
    NSData *data = _builtinThemes[themeId];
    if (!data) {
        NSArray *info = _localThemes[themeId];
        data = (info.count >= 2) ? info[1] : nil;
    }
    if (data) {
        NSDictionary *json = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
        return [@"dark" isEqualToString:json[@"type"]];
    }
    return NO;
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

+ (NSData*)jsonDataForTheme:(NSString*)themeId {
    ASCThemesController *instance = [self sharedInstance];
    NSData *data = instance.builtinThemes[themeId];
    if (!data) {
        NSArray *info = instance.localThemes[themeId];
        data = (info.count >= 2) ? info[1] : nil;
    }
    return data;
}

+ (NSColor*)colorFromValues:(NSDictionary*)values themeType:(NSString*)themeType forName:(NSString*)name {
    if ([name isEqualToString:tabActiveTextColor]) {
        NSString *tabThemeType = values[@"tab-editor-theme-type"] ?: @"dark";
        BOOL isAppDark = [@"dark" isEqualToString:themeType];
        BOOL isTabDark = [@"dark" isEqualToString:tabThemeType];
        NSString *colorKey = (isTabDark == isAppDark) ? @"tab-simple-active-text" : @"text-inverse";
        return colorFromHexString(values[colorKey]);
    }

    NSString *key = jsonKeysForColorName(name);
    id val = values[key];
    if (val && [val isKindOfClass:[NSString class]] && [(NSString*)val length] > 0) {
        return colorFromHexString(val);
    }
    return nil;
}

+ (NSColor*)colorFromTheme:(NSString*)themeId forName:(NSString*)name {
    NSData *jsonData = [self jsonDataForTheme:themeId];
    if (!jsonData) return nil;

    NSDictionary *json = [NSJSONSerialization JSONObjectWithData:jsonData options:0 error:nil];
    if (!json) return nil;

    NSDictionary *values = json[@"values"];
    if (![values isKindOfClass:[NSDictionary class]]) {
        values = json[@"colors"];
    }
    if (![values isKindOfClass:[NSDictionary class]]) return nil;

    return [self colorFromValues:values themeType:json[@"type"] forName:name];
}

+ (NSString*)currentThemeId {
    return [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];
}

+ (BOOL)isCurrentThemeDark {
    NSString * theme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme];
    if ([uiThemeSystem isEqualToString:theme]) {
        return [self isSystemDarkMode];
    }

    NSData *jsonData = [self jsonDataForTheme:theme];
    if (jsonData) {
        NSDictionary *json = [NSJSONSerialization JSONObjectWithData:jsonData options:0 error:nil];
        return [@"dark" isEqualToString:json[@"type"]];
    }

    return NO;
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

    return [self colorFromTheme:theme forName:name];
}

+ (BOOL)isSystemDarkMode {
    if (@available(macOS 10.14, *)) {
        NSString * appleInterfaceStyle = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];

        if (appleInterfaceStyle && [appleInterfaceStyle length] > 0) {
            return [[appleInterfaceStyle lowercaseString] containsString:@"dark"];
        }
    }

    return NO;
}

+ (BOOL)isDarkWindowAppearance {
    if (@available(macOS 10.14, *)) {
        return [ASCThemesController isCurrentThemeDark];
    }
    return [NSApplication isSystemDarkMode];
}

+ (NSString*)userThemesPath {
    NSString *appSupport = NSSearchPathForDirectoriesInDomains(
        NSApplicationSupportDirectory, NSUserDomainMask, YES).firstObject;
    return [appSupport stringByAppendingPathComponent:@"ONLYOFFICE/uithemes"];
}

+ (BOOL)validateTheme:(NSDictionary*)jsonObj {
    if (!jsonObj[@"id"] || !jsonObj[@"name"]) return NO;

    NSString *themeId = jsonObj[@"id"];
    if (![themeId isKindOfClass:[NSString class]] || themeId.length == 0) return NO;

    NSError *err = nil;
    NSRegularExpression *re = [NSRegularExpression
        regularExpressionWithPattern:@"[^\\w\\d\\-]"
                             options:0
                               error:&err];
    if (!err && [re numberOfMatchesInString:themeId
                                    options:0
                                      range:NSMakeRange(0, themeId.length)] > 0) {
        return NO;
    }

    NSDictionary *colors = jsonObj[@"colors"] ?: jsonObj[@"values"];
    if ([colors isKindOfClass:[NSDictionary class]]) {
        for (NSString *key in colors) {
            if (!err && [re numberOfMatchesInString:key
                                            options:0
                                              range:NSMakeRange(0, key.length)] > 0) {
                return NO;
            }
        }
    }

    return YES;
}

+ (BOOL)isLocalTheme:(NSString*)themeId {
    if (!themeId) return NO;
    ASCThemesController *instance = [self sharedInstance];
    return instance.builtinThemes[themeId] == nil;
}

+ (BOOL)addLocalTheme:(NSDictionary*)jsonObj filePath:(NSString*)filePath {
    if (![self validateTheme:jsonObj]) return NO;

    NSString *destDir = [self userThemesPath];
    NSFileManager *fm = [NSFileManager defaultManager];

    NSError *err = nil;
    if (![fm createDirectoryAtPath:destDir withIntermediateDirectories:YES attributes:nil error:&err]) {
        NSLog(@"ASCThemesController: failed to create themes dir: %@", err);
        return NO;
    }

    NSString *fileName = [filePath lastPathComponent];
    NSString *destPath = [destDir stringByAppendingPathComponent:fileName];

    if ([fm fileExistsAtPath:destPath]) {
        [fm removeItemAtPath:destPath error:nil];
    }

    NSData *outData = [NSJSONSerialization dataWithJSONObject:jsonObj options:0 error:&err];
    if (err || !outData) {
        NSLog(@"ASCThemesController: failed to serialize theme json: %@", err);
        return NO;
    }

    if (![outData writeToFile:destPath options:NSDataWritingAtomic error:&err]) {
        NSLog(@"ASCThemesController: failed to write theme file: %@", err);
        return NO;
    }

    NSString *themeId = jsonObj[@"id"];
    ASCThemesController *instance = [self sharedInstance];
    instance.localThemes[themeId] = @[destPath, outData];
    return YES;
}

+ (NSArray*)localThemesToJson {
    ASCThemesController *instance = [self sharedInstance];
    NSDictionary<NSString*, NSArray*> *localThemes = instance.localThemes;
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:localThemes.count];

    for (NSArray *info in localThemes.allValues) {
        if (info.count < 2) continue;
        NSError *err = nil;
        id obj = [NSJSONSerialization JSONObjectWithData:info[1] options:0 error:&err];
        if (!err && [obj isKindOfClass:[NSDictionary class]]) {
            [result addObject:obj];
        }
    }
    return [result copy];
}

+ (BOOL)containsTheme:(NSString*)themeId {
    if (!themeId) return NO;
    ASCThemesController *instance = [self sharedInstance];
    return instance.builtinThemes[themeId] != nil ||
           instance.localThemes[themeId] != nil;
}

+ (BOOL)checkDestinationThemeFileExist:(NSString*)srcPath {
    NSString *fileName = [srcPath lastPathComponent];
    NSString *destPath = [[self userThemesPath] stringByAppendingPathComponent:fileName];
    return [[NSFileManager defaultManager] fileExistsAtPath:destPath];
}

@end

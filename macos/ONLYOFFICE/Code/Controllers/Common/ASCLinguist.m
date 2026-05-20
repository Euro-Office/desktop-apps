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
//  ASCLinguist.m
//  ONLYOFFICE
//
//  Created by Maxim Kadushkin on 03.02.2022.
//  Copyright © 2022 Ascensio System SIA. All rights reserved.
//

#import "ASCLinguist.h"
#import "ASCConstants.h"
#import <Carbon/Carbon.h>

@implementation ASCLinguist

static BOOL uiLayoutDirectionRTL = NO;

+ (void)init {
    [[NSUserDefaults standardUserDefaults] setObject:[NSArray arrayWithObject:[ASCLinguist appLanguageCode]] forKey:@"AppleLanguages"];
    [[NSUserDefaults standardUserDefaults] setObject:[ASCLinguist appLanguageCode] forKey:@"AppleLocale"];
    [[NSUserDefaults standardUserDefaults] synchronize];

    uiLayoutDirectionRTL = [NSLocale characterDirectionForLanguage:[ASCLinguist appLanguageCode]] == NSLocaleLanguageDirectionRightToLeft;
    if ( uiLayoutDirectionRTL ) {
        [[NSUserDefaults standardUserDefaults] setObject:@"YES" forKey:@"AppleTextDirection"];
        [[NSUserDefaults standardUserDefaults] setObject:@"YES" forKey:@"NSForceRightToLeftWritingDirection"];

        [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"NSForceLeftToRightWritingDirection"];
    } else {
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"AppleTextDirection"];
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:@"NSForceRightToLeftWritingDirection"];

        [[NSUserDefaults standardUserDefaults] setObject:@"YES" forKey:@"NSForceLeftToRightWritingDirection"];
    }
}

+ (NSString *)appLanguageCode {
    NSString * uiLang = [[NSUserDefaults standardUserDefaults] objectForKey:ASCUserUILanguage];
    if ( !uiLang )
        uiLang = [[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode];

    if ( [uiLang length] < 3 )
        uiLang = [NSString stringWithFormat:@"%@-%@", [uiLang lowercaseString], [uiLang uppercaseString]];

    return uiLang;
}

+ (void)setAppLanguageCode:(NSString *)langCode {
    [[NSUserDefaults standardUserDefaults] setObject:langCode forKey:ASCUserUILanguage];
    [[NSUserDefaults standardUserDefaults] setObject:[NSArray arrayWithObject:langCode] forKey:@"AppleLanguages"];
    [[NSUserDefaults standardUserDefaults] setObject:langCode forKey:@"AppleLocale"];

    [[NSUserDefaults standardUserDefaults] synchronize];
}

+ (BOOL)isUILayoutDirectionRtl {
    return uiLayoutDirectionRTL;
}

+ (NSDictionary *)keyboardLanguages {
    NSMutableDictionary * outdict = [NSMutableDictionary dictionary];
    CFArrayRef sourceList = TISCreateInputSourceList(NULL, false);
    if ( sourceList ) {
        CFIndex count = CFArrayGetCount(sourceList);
        for (CFIndex i = 0; i < count; i++) {
            TISInputSourceRef source = (TISInputSourceRef)CFArrayGetValueAtIndex(sourceList, i);

            // Get only keyboard input sources
            CFTypeRef type = TISGetInputSourceProperty(source, kTISPropertyInputSourceType);
            if ( CFEqual(type, kTISTypeKeyboardLayout) || CFEqual(type, kTISTypeKeyboardInputMode) ) {
                // Get the language codes (ISO 639 format)
                CFArrayRef languages = TISGetInputSourceProperty(source, kTISPropertyInputSourceLanguages);
                if ( languages && CFArrayGetCount(languages) > 0 ) {
                    // Get the first language code (usually the primary one)
                    CFStringRef firstLang = CFArrayGetValueAtIndex(languages, 0);
                    NSString *langString = (__bridge NSString *)firstLang;

                    NSString *name = nil;
                    NSLocale *locale = [NSLocale localeWithLocaleIdentifier:langString];
                    if (locale) {
                        name = [locale displayNameForKey:NSLocaleIdentifier value:langString];
                        if (name) {
                            name = [name capitalizedString];
                        }
                    }
                    
                    if (!name) {
                        CFStringRef localizedName = (CFStringRef)TISGetInputSourceProperty(source, kTISPropertyLocalizedName);
                        name = (__bridge NSString *)localizedName;
                    }
                    outdict[langString] = name;
//                    NSArray *components = [langString componentsSeparatedByString:@"-"];
//                    NSString *languageCode = components[0];
//                    if (components.count > 1) {
//                        NSString *region = components[1];
//                        NSLog(@"Language: %@ (ISO code: %@-%@)", name, languageCode, region);
//                    }
                }
            }
        }

        CFRelease(sourceList);
    }

    return outdict;
}

+ (NSDictionary *)availableLanguages {
    return @{
        @"en-US": @{@"name": @"English (United States)", @"enname": @""},
        @"en-GB": @{@"name": @"English (United Kingdom)", @"enname": @""},
        @"ru": @{@"name": @"Русский", @"enname": @"Russian"},
        @"de": @{@"name": @"Deutsch", @"enname": @"German"},
        @"fr": @{@"name": @"Français", @"enname": @"French"},
        @"es": @{@"name": @"Español", @"enname": @"Spanish"},
        @"it": @{@"name": @"Italiano", @"enname": @"Italian"},
        @"pt-BR": @{@"name": @"Português Brasileiro", @"enname": @"Portuguese (Brazil)"},
        @"pt-PT": @{@"name": @"Português (Portugal)", @"enname": @"Portuguese (Portugal)"},
        @"zh-CN": @{@"name": @"简体中文", @"enname": @"Chinese (Simplified)"},
        @"zh-TW": @{@"name": @"繁體中文", @"enname": @"Chinese (Traditional)"},
        @"sk-SK": @{@"name": @"Slovenčina", @"enname": @"Slovak"},
        @"cs-CZ": @{@"name": @"Čeština", @"enname": @"Czech"},
        @"pl-PL": @{@"name": @"Polski", @"enname":@"Polish"},
        @"ca-ES": @{@"name": @"Catalan", @"enname": @"Catalan"},
        @"da-DK": @{@"name": @"Dansk", @"enname": @"Danish"},
        @"el-GR": @{@"name": @"Ελληνικά", @"enname": @"Greek"},
        // @"et-EE": @"Eesti",
        @"fi-FI": @{@"name": @"Suomi", @"enname": @"Finnish"},
        // @"ga-IE": @"Gaeilge",
        // @"hi-IN": @"हिन्दी",
        @"hr-HR": @{@"name": @"Hrvatska", @"enname": @"Croatian"},
        @"hu-HU": @{@"name": @"Magyar", @"enname": @"Hungarian"},
        @"hy-AM": @{@"name": @"Հայերեն", @"enname": @"Armenian"},
        @"id-ID": @{@"name": @"Indonesian", @"enname": @"Indonesian"},
        @"no-NO": @{@"name": @"Norsk", @"enname": @"Norwegian"},
        @"ro-RO": @{@"name": @"Romanian", @"enname": @"Romanian"},
        @"sl-SI": @{@"name": @"Slovene", @"enname": @"Slovenian"},
        @"sv-SE": @{@"name": @"Svenska", @"enname": @"Swedish"},
        @"sr-Latn-RS": @{@"name": @"Srpski (Latin)", @"enname": @"Serbian (Latin)"},
        @"sr-Cyrl-RS": @{@"name": @"Српски (ћирилица)", @"enname": @"Serbian (Cyrillic)"},
        @"tr-TR": @{@"name": @"Türkçe", @"enname": @"Turkish"},
        @"ja-JP": @{@"name": @"日本語", @"enname": @"Japanese"},
        @"ko-KR": @{@"name": @"한국어", @"enname": @"Korean"},
        @"bg-BG": @{@"name": @"Български", @"enname": @"Bulgarian"},
        @"nl-NL": @{@"name": @"Nederlands", @"enname": @"Dutch"},
        @"vi-VN": @{@"name": @"Tiếng Việt", @"enname": @"Vietnamese"},
        @"lv-LV": @{@"name": @"Latviešu valoda", @"enname": @"Latvian"},
        // @"lt-LT": @"Lietuvių kalba",
        @"be-BY": @{@"name": @"Беларуская мова", @"enname": @"Belarusian"},
        @"uk-UA": @{@"name": @"Украї́нська мо́ва", @"enname": @"Ukrainian"},
        @"lo-LA": @{@"name": @"ພາສາລາວ", @"enname": @"Lao"},
        @"gl-ES": @{@"name": @"Galego", @"enname": @"Galego"},
        @"si-LK": @{@"name": @"සිංහල", @"enname": @"Sinhala (Sri Lanka)"},
        @"ar-SA": @{@"name": @"اَلْعَرَبِيَّة", @"enname": @"Arabic"},
        @"sq-AL": @{@"name": @"Shqip", @"enname": @"Albanian"},
        @"he-IL": @{@"name": @"עברית", @"enname": @"Hebrew"},
        @"ur-PK": @{@"name": @"اردو", @"enname": @"Urdu"}
    };
}

@end

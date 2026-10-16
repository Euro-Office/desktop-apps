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
//  ASCAboutController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 18.02.16.
//  Copyright © 2017 Ascensio System SIA. All rights reserved.
//

#import "ASCAboutController.h"

#import "ASCConstants.h"
#import "ASCExternalController.h"
#import "ASCSharedSettings.h"
#import "ASCLicenseController.h"

@interface ASCAboutController () {
    BOOL isCommercialVersion;
}
@property (weak) IBOutlet NSTextField *appNameText;
@property (weak) IBOutlet NSTextField *versionText;
@property (weak) IBOutlet NSTextField *copyrightText;
@property (weak) IBOutlet NSButton *licenseButton;
@property (weak) IBOutlet NSStackView *infoStackView;
@end

@implementation ASCAboutController

/// {"buildInfo": html, "legal": html}, rendered at build time by the "Render About text" phase
+ (NSDictionary *)aboutHtml {
    NSURL * url = [[NSBundle mainBundle] URLForResource:@"about" withExtension:@"json"];
    NSData * data = url ? [NSData dataWithContentsOfURL:url] : nil;
    id about = data ? [NSJSONSerialization JSONObjectWithData:data options:0 error:nil] : nil;
    return [about isKindOfClass:[NSDictionary class]] ? about : @{};
}

+ (NSMutableAttributedString *)attributedStringFromHtml:(NSString *)html {
    NSData * data = [(html ?: @"") dataUsingEncoding:NSUTF8StringEncoding];
    return [[NSMutableAttributedString alloc] initWithHTML:data
                                                   options:@{NSCharacterEncodingDocumentOption: @(NSUTF8StringEncoding)}
                                        documentAttributes:nil];
}

- (NSString *)buildSuffix {
    // Plain text: the version label toggles on click, so it can't host links
    return [[ASCAboutController attributedStringFromHtml:[ASCAboutController aboutHtml][@"buildInfo"]] string];
}

- (NSAttributedString *)legalText {
    // #888888 marks headings so they can be mapped to the dynamic secondary colour below
    NSString * style = @"<style>body{font:12px -apple-system,sans-serif}p{margin:0 0 8px}"
                        ".about-legal-heading{font-weight:bold;text-transform:uppercase;letter-spacing:.05em;color:#888888;margin:12px 0 4px}"
                        ".about-legal-commits{border-collapse:collapse}.about-legal-commits td{padding:1px 16px 1px 0}</style>";
    NSString * legal = [ASCAboutController aboutHtml][@"legal"];
    NSMutableAttributedString * text = [ASCAboutController attributedStringFromHtml:[style stringByAppendingString:legal ?: @""]];

    NSColor * headingMarker = [NSColor colorWithSRGBRed:0x88/255.0 green:0x88/255.0 blue:0x88/255.0 alpha:1];
    [text enumerateAttribute:NSForegroundColorAttributeName inRange:NSMakeRange(0, text.length) options:0 usingBlock:^(NSColor * color, NSRange range, BOOL * stop) {
        BOOL isHeading = [[color colorUsingColorSpace:[NSColorSpace sRGBColorSpace]] isEqual:headingMarker];
        [text addAttribute:NSForegroundColorAttributeName value:isHeading ? [NSColor secondaryLabelColor] : [NSColor labelColor] range:range];
    }];
    return text;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
    
    NSDictionary * infoDictionary = [[NSBundle mainBundle] infoDictionary];
    NSDictionary * localizedInfoDictionary = [[NSBundle mainBundle] localizedInfoDictionary];
    
    NSString * locProductName   = [ASCHelper appName];
    NSString * locCopyright     = localizedInfoDictionary[@"NSHumanReadableCopyright"];

    locCopyright = locCopyright ? locCopyright : infoDictionary[@"NSHumanReadableCopyright"];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onCommercialInfo)]) {
        NSString * commercialInfo = [externalDelegate onCommercialInfo];

        if (commercialInfo) {
            NSTextField * commercialTextField;
            if (@available(macOS 10.12, *)) {
                commercialTextField = [NSTextField labelWithString:commercialInfo];
            } else {
                commercialTextField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, self.infoStackView.frame.size.width, 35)];
                [commercialTextField setStringValue:commercialInfo];
                [commercialTextField setFont:[NSFont systemFontOfSize:[NSFont systemFontSize]]];
                [commercialTextField setBezeled:NO];
                [commercialTextField setDrawsBackground:NO];
                [commercialTextField setEditable:NO];
                [commercialTextField setSelectable:NO];
            }
            [commercialTextField setAlignment:NSTextAlignmentCenter];
            [commercialTextField setLineBreakMode:NSLineBreakByWordWrapping];
            [commercialTextField setUsesSingleLineMode:NO];

            [self.infoStackView insertArrangedSubview:commercialTextField atIndex:2];
        }
    }

    NSURL * eulaUrl = [[NSBundle mainBundle] URLForResource:@"EULA" withExtension:@"html" subdirectory:@"license"];
    isCommercialVersion = eulaUrl != nil;

        // About View
        // Setup license button view
        NSMutableAttributedString * attrTitle = [[NSMutableAttributedString alloc] initWithAttributedString:[self.licenseButton attributedTitle]];
        long len = [attrTitle length];
        NSRange range = NSMakeRange(0, len);
        [attrTitle addAttribute:NSForegroundColorAttributeName value:[NSColor linkColor] range:range];
        [attrTitle fixAttributesInRange:range];
        [self.licenseButton setAttributedTitle:attrTitle];
        
#ifdef _MAS
        [self.licenseButton setHidden:YES];
#endif
        
        // Product name
        [self.appNameText setStringValue:locProductName];
        
        // Version
        NSString * tplVersion = !isCommercialVersion ? NSLocalizedString(@"Community version %@", nil) : NSLocalizedString(@"Enterprise version %@", nil);
        [self.versionText setStringValue:[[NSString stringWithFormat:tplVersion, [infoDictionary objectForKey:@"CFBundleShortVersionString"]] stringByAppendingString:[self buildSuffix]]];

        NSClickGestureRecognizer *click = [[NSClickGestureRecognizer alloc] initWithTarget:self action:@selector(onVersionClick:)];
        [self.versionText addGestureRecognizer:click];
        
        // If has extra features
        if ([[[ASCSharedSettings sharedInstance] settingByKey:kSettingsHasExtraFeatures] boolValue]) {
            [self.versionText setStringValue:[NSString stringWithFormat:@"%@\n%@",
                                              self.versionText.stringValue,
                                              NSLocalizedString(@"With access to pro features", nil)]];
        }
        
        // Copyright and legal notices
        [self.copyrightText setStringValue:locCopyright];
        [self.copyrightText setHidden:YES];

        NSScrollView * legalScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 350, 220)];
        legalScroll.hasVerticalScroller = YES;
        legalScroll.drawsBackground = NO;
        legalScroll.borderType = NSNoBorder;
        legalScroll.translatesAutoresizingMaskIntoConstraints = NO;

        NSTextView * legalView = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, legalScroll.contentSize.width, 220)];
        legalView.editable = NO;
        legalView.selectable = YES;
        legalView.drawsBackground = NO;
        legalView.autoresizingMask = NSViewWidthSizable;
        legalView.textContainer.widthTracksTextView = YES;
        [legalView.textStorage setAttributedString:[self legalText]];
        legalScroll.documentView = legalView;

        [self.infoStackView addArrangedSubview:legalScroll];
        [legalScroll.widthAnchor constraintEqualToConstant:350].active = YES;
        [legalScroll.heightAnchor constraintEqualToConstant:220].active = YES;
        
        // Window
        [self setTitle:[NSString stringWithFormat:NSLocalizedString(@"About %@", nil), locProductName]];
}

- (void)viewDidAppear {
    [super viewDidAppear];
        
    [self.view.window setStyleMask:[self.view.window styleMask] & ~NSResizableWindowMask];
}

- (void)viewDidDisappear {
    [super viewDidDisappear];
    
    [NSApp stopModal];
}

- (void)onVersionClick:(NSTextField *)sender {
    NSDictionary * infoDictionary = [[NSBundle mainBundle] infoDictionary];

    NSString * tplVersion = !isCommercialVersion ? NSLocalizedString(@"Community version %@ (%@-%@)", nil) :
                                            NSLocalizedString(@"Enterprise version %@ (%@-%@)", nil);
    [self.versionText setStringValue:[NSString stringWithFormat:tplVersion,
                                      [infoDictionary objectForKey:@"CFBundleShortVersionString"],
                                      [infoDictionary objectForKey:@"CFBundleVersion"],
                                      [infoDictionary objectForKey:@"ASCBundleBuildNumber"]]];
    
#if _V8_VERSION
    [self.versionText setStringValue:[NSString stringWithFormat:@"%@ x86", [self.versionText stringValue]]];
#elif _X86_64_ONLY
    [self.versionText setStringValue:[NSString stringWithFormat:@"%@ x86_64", [self.versionText stringValue]]];
#elif _ARM_ONLY
    [self.versionText setStringValue:[NSString stringWithFormat:@"%@ Apple Silicon", [self.versionText stringValue]]];
#endif
    [self.versionText setStringValue:[self.versionText.stringValue stringByAppendingString:[self buildSuffix]]];
}

- (IBAction)onLicenseButtonClick:(id)sender {
    NSURL *licenseUrl = [NSURL URLWithString:@"https://github.com/Euro-Office/DesktopEditors/blob/main/LICENSE"];
    [[NSWorkspace sharedWorkspace] openURL:licenseUrl];
}

@end

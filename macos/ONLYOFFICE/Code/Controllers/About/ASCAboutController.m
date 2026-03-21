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
        [self.versionText setStringValue:[NSString stringWithFormat:tplVersion, [infoDictionary objectForKey:@"CFBundleShortVersionString"]]];

        NSClickGestureRecognizer *click = [[NSClickGestureRecognizer alloc] initWithTarget:self action:@selector(onVersionClick:)];
        [self.versionText addGestureRecognizer:click];
        
        // If has extra features
        if ([[[ASCSharedSettings sharedInstance] settingByKey:kSettingsHasExtraFeatures] boolValue]) {
            [self.versionText setStringValue:[NSString stringWithFormat:@"%@\n%@",
                                              self.versionText.stringValue,
                                              NSLocalizedString(@"With access to pro features", nil)]];
        }
        
        // Copyright
        [self.copyrightText setStringValue:locCopyright];
        
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
}

- (IBAction)onLicenseButtonClick:(id)sender {
    NSURL * eulaUrl = [[NSBundle mainBundle] URLForResource:@"EULA" withExtension:@"html" subdirectory:@"license"];
    if ( !eulaUrl )
        eulaUrl = [[NSBundle mainBundle] URLForResource:@"LICENSE" withExtension:@"html" subdirectory:@"license"];
    
    NSWindowController * windowController = [self.storyboard instantiateControllerWithIdentifier:@"ASCLicenseWindowControllerId"];
    ASCLicenseController *licView = (ASCLicenseController *)windowController.contentViewController;
    [licView setUrl:eulaUrl];
    NSWindow *licWindow = windowController.window;
    
    NSRect parentFrame = self.view.window.frame;
    NSRect childFrame = licWindow.frame;
    [licWindow setFrameOrigin:NSMakePoint(NSMidX(parentFrame) - childFrame.size.width/2,
                                          NSMidY(parentFrame) - childFrame.size.height/2)];
    [licWindow makeKeyAndOrderFront:nil]; // Show the window first to apply the coordinates
    
    [NSApp runModalForWindow:licWindow];
}

@end

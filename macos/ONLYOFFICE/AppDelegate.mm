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
//  AppDelegate.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "AppDelegate.h"
#import "ASCConstants.h"
#import "ASCSavePanelWithFormatController.h"
#import "ASCCertificatePreviewController.h"
#import "ASCCertificateQLPreviewController.h"
#import "ASCTitleWindowController.h"
#import "ASCCommonViewController.h"
#import "ASCEditorWindowController.h"
#import "ASCEditorWindow.h"
#import "ASCTitleWindow.h"
#import "ASCSharedSettings.h"
#import "ASCTabView.h"
#import "NSString+Extensions.h"
#import "NSCefView.h"
#import "ASCHelper.h"
#import "AnalyticsHelper.h"
#import "ASCExternalController.h"
#import "ASCEditorJSVariables.h"
#import "ASCThemesController.h"
#import "NSAlert+SynchronousSheet.h"
#import "NSString+Extensions.h"
#import "NSDictionary+Extensions.h"
#import "PureLayout.h"
#import "ascprinter.h"

#ifndef _MAS
    #import "PFMoveApplication.h"
#endif

@interface AppDelegate () {
    ASCPrinterContext * m_pContext;
    NSTimer *dropTimer;
    NSPoint lastCursorPos;
}
@property (weak) IBOutlet NSMenuItem *updateMenuItem;
@property (weak) IBOutlet NSMenuItem *eulaMenuItem;
@property (nonatomic, weak) NSWindow *dropEditorWindow;
@property (nonatomic, assign) BOOL terminationAlreadyHandled;
@property (nonatomic, assign) BOOL hasFilesToOpenOnLaunch;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    
    if (@available(macOS 10.12, *)) {
        [NSWindow setAllowsAutomaticWindowTabbing:NO];
    }
    
    self.editorWindowControllers = [NSMutableArray array];
    
    [self updateAppAppearance];
    
#ifndef _MAS
    PFMoveToApplicationsFolderIfNecessary();
#endif
    
    // Remove 'Start Dictation' and 'Special Characters' from menu
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSDisabledDictationMenuItem"];
    [[NSUserDefaults standardUserDefaults] setBool:NO forKey:@"NSDisabledCharacterPaletteMenuItem"];
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    
    void (^addObserverFor)(_Nullable NSNotificationName, SEL) = ^(_Nullable NSNotificationName name, SEL selector) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:selector
                                                     name:name
                                                   object:nil];
    };
    
    addObserverFor(ASCEventNameChangedUITheme, @selector(onUIThemeChanged:));
    addObserverFor(ASCEventNameChangedSystemTheme, @selector(onSystemThemeChanged:));
    addObserverFor(CEFEventNameCreateTab, @selector(onCEFCreateTab:));
    addObserverFor(CEFEventNameStartSaveDialog, @selector(onCEFStartSave:));
    addObserverFor(CEFEventNameSaveLocal, @selector(onCEFSaveLocalFile:));
    addObserverFor(CEFEventNameOpenLocalFile, @selector(onCEFOnOpenLocalFile:));
    addObserverFor(CEFEventNameEditorEvent, @selector(onCEFEditorEvent:));
    addObserverFor(CEFEventNameOpenImage, @selector(onCEFOpenLocalImage:));
    addObserverFor(CEFEventNameOpenFileDialog, @selector(onCEFOpenFileDialog:));
    addObserverFor(CEFEventNameFileInFinder, @selector(onCEFFileInFinder:));
    addObserverFor(CEFEventNameCertificatePreview, @selector(onCEFCertificatePreview:));
    addObserverFor(CEFEventNameEditorDocumentReady, @selector(onCEFEditorDocumentReady:));
    addObserverFor(CEFEventNameKeyboardDown, @selector(onCEFKeyDown:));
    addObserverFor(CEFEventNameSaveBeforSign, @selector(onCEFSaveBeforeSign:));
    addObserverFor(CEFEventNamePrintDialog, @selector(onCEFOnBeforePrintEnd:));
    addObserverFor(ASCEventNameRecoveryFiles, @selector(onRecoveryFiles:));
    addObserverFor(ASCEventNameEditorWindowMoving, @selector(onEditorWindowMoving:));
        
    // Google Analytics
    
#ifdef _PRODUCT_ONLYOFFICE
//    [[AnalyticsHelper sharedInstance] beginPeriodicReportingWithAccount:@"UA-XXXXXXXXX-1"
//                                                                   name:@"ONLYOFFICE"
//                                                                version:[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"]];
#endif
    
    
//    // Hotkey conflict resolve
//    [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask handler:^NSEvent * _Nullable(NSEvent * _Nonnull event) {
//        NSEventModifierFlags flags = [event modifierFlags] & NSDeviceIndependentModifierFlagsMask;
//        if((flags - (NSShiftKeyMask | NSControlKeyMask) == 0) && ([event keyCode] == 2)) { // Shift+Ctrl+D
//            //
//        }
//        
//        return event;
//    }];

    NSArray * arguments = [[NSProcessInfo processInfo] arguments];
    NSArray * keysCreateNew{@[@"word",@"cell",@"slide",@"form"]};
    for (NSString * arg in arguments) {
        if ( [arg hasPrefix:@"--new:"] || [arg hasPrefix:@"--new="] ) {
            NSString * param = [arg substringFromIndex:6];

            if ( [keysCreateNew containsObject:param] ) {
                self.hasFilesToOpenOnLaunch = YES;
                
                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                    object:nil
                                                                  userInfo:@{ @"action"  : @(ASCTabActionCreateLocalFile),
                                                                              @"type"    : param,
                                                                              @"active"  : @(YES) }];
            }
        } else if ([arg isEqualToString:@"--lock-portals"]) {
            [[NSUserDefaults standardUserDefaults] setBool:YES forKey:ASCUserLockPageConnections];
        } else if ([arg isEqualToString:@"--unlock-portals"]) {
            [[NSUserDefaults standardUserDefaults] removeObjectForKey:ASCUserLockPageConnections];
        }
    }
    
    if (!self.hasFilesToOpenOnLaunch) {
        [self presentMainWindow];
    }
}

/// If your delegate implements this method, AppKit does not call the application(_:openFile:)
/// or application(_:openFiles:) methods.
- (void)application:(NSApplication *)application openURLs:(NSArray<NSURL *> *)urls {

    /// Handle links
    
    NSMutableArray<NSURL *> * openLinks = @[].mutableCopy;
    
    [urls enumerateObjectsUsingBlock:^(NSURL * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        if ([obj.scheme isEqualToString:kSchemeApp]) {
            NSString * strLink = [obj.absoluteString stringByRemovingPercentEncoding];
            
            NSString * actionSelectPanel = [NSString stringWithFormat:@"%@://%@|", kSchemeApp, @"action|panel"];
            NSString * actionInstallPlugin = [NSString stringWithFormat:@"%@://%@|", kSchemeApp, @"action|install-plugin"];
            NSString * appCommand = [NSString stringWithFormat:@"%@://%@|", kSchemeApp, @"command/"];
            if ( [strLink hasPrefix:actionSelectPanel] ) {
                NSString * panelName = [strLink substringFromIndex:actionSelectPanel.length];
                
                if ( panelName.length ) {
                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                    pCommand->put_Command(L"panel:select");
                    pCommand->put_Param([panelName stdwstring]);

                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                    pEvent->m_pData = pCommand;

                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                    appManager->SetEventToAllMainWindows(pEvent);
                }
            } else
            if ( [strLink hasPrefix:actionInstallPlugin] ) {
                NSString * pluginName = [strLink substringFromIndex:actionInstallPlugin.length];

                CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                appManager->InstallPluginFromStore([pluginName stdwstring]);
            } else
            if ( [strLink hasPrefix:appCommand] ) {
                NSString * cmd = [strLink substringFromIndex:appCommand.length];

                CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                appManager->CallCommand([cmd stdwstring]);
            } else {
                [openLinks addObject:obj];
            }
        }
    }];
    
    if (openLinks.count > 0) {
        if (NSArray<NSURL *> * storedAppLinks = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsOpenAppLinks]) {
            NSMutableArray * extendArrayLinks = [storedAppLinks mutableCopy];
            [extendArrayLinks addObjectsFromArray:openLinks];
            [[ASCSharedSettings sharedInstance] setSetting:extendArrayLinks forKey:kSettingsOpenAppLinks];
        } else {
            [[ASCSharedSettings sharedInstance] setSetting:openLinks forKey:kSettingsOpenAppLinks];
        }
        
        [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameOpenAppLinks
                                                            object:[[ASCSharedSettings sharedInstance] settingByKey:kSettingsOpenAppLinks]
                                                          userInfo:nil];
    }
    
    /// Handle files
    
    NSMutableArray<NSString *> * fileNames = @[].mutableCopy;
    
    for (NSURL * url in urls) {
        if ([url isFileURL]) {
            [fileNames addObject:[url path]];
        }
    }
    
    [self application:application openFiles:fileNames];
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename {
    return YES;
}

- (void)application:(NSApplication *)sender openFiles:(NSArray<NSString *> *)filenames {
    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onShouldOpenFile:)]) {
        NSMutableArray<NSString *> * processedFileList = [NSMutableArray array];

        for (NSString * filePath in filenames) {
            if ([externalDelegate onShouldOpenFile:filePath]) {
                [processedFileList addObject:filePath];
            }
        }

        filenames = processedFileList;
    }

    if (filenames.count > 0) {
        self.hasFilesToOpenOnLaunch = YES;
    }

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.1 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        for (NSString * filePath in filenames) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                                                                         @"action"  : @(ASCTabActionOpenLocalFile),
                                                                         @"path"    : filePath,
                                                                         @"active"  : @(YES),
                                                                         @"external": @(YES)
                                                                         }];
        }
    });
}

-(NSMenu *)applicationDockMenu:(NSApplication *)sender {
    NSMenu * menu = [[NSMenu alloc] initWithTitle:@"dockNewMenu"];

    NSString * item_text = NSLocalizedStringWithDefaultValue(@"new-document", @"Localizable", [NSBundle mainBundle], @"New Document", nil);
    NSMenuItem *
    itemNewDoc = [[NSMenuItem alloc] initWithTitle:item_text
                                            action:@selector(onMenuNew:)
                                     keyEquivalent:@""];
    [itemNewDoc setTag: 0];
    [menu addItem: itemNewDoc];
    
    item_text = NSLocalizedStringWithDefaultValue(@"new-spreadsheet", @"Localizable", [NSBundle mainBundle], @"New Spreadsheet", nil);
    itemNewDoc = [[NSMenuItem alloc] initWithTitle:item_text
                                            action:@selector(onMenuNew:)
                                     keyEquivalent:@""];
    [itemNewDoc setTag: 2];
    [menu addItem: itemNewDoc];
    
    item_text = NSLocalizedStringWithDefaultValue(@"new-presentation", @"Localizable", [NSBundle mainBundle], @"New Presentation", nil);
    itemNewDoc = [[NSMenuItem alloc] initWithTitle:item_text
                                            action:@selector(onMenuNew:)
                                     keyEquivalent:@""];
    [itemNewDoc setTag: 1];
    [menu addItem: itemNewDoc];
    
    item_text = NSLocalizedStringWithDefaultValue(@"new-pdfform", @"Localizable", [NSBundle mainBundle], @"New PDF Form", nil);
    itemNewDoc = [[NSMenuItem alloc] initWithTitle:item_text
                                            action:@selector(onMenuNew:)
                                     keyEquivalent:@""];
    [itemNewDoc setTag: 3];
    [menu addItem: itemNewDoc];

    return menu;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application

#ifdef _PRODUCT_ONLYOFFICE
    [[AnalyticsHelper sharedInstance] handleApplicationWillClose];
    [[NSUserDefaults standardUserDefaults] synchronize];
#endif
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)theApplication {
    if (self.terminationAlreadyHandled) {
        return NSTerminateNow;
    }
    
    if (![self shouldTerminateApplication])
        return NSTerminateCancel;
    
    self.terminationAlreadyHandled = YES;
    return NSTerminateNow;
}

- (BOOL)preferOpenEditorWindow {
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    auto mode = appManager->GetUserSettings()->Get(L"editor-window-mode");
    return (mode == L"1") ? YES : NO;
}

- (BOOL)shouldTerminateApplication {
    NSInteger unsaved = 0;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO),
                                                                 @"terminate"  : @(YES)
                                                               }];
    NSMutableArray * locked_uuids = [NSMutableArray array];
    
    ASCCommonViewController * controller = nil;
    for (NSWindow *window in [NSApp windows]) {
        if ([window isKindOfClass:[ASCTitleWindow class]]) {
            controller = (ASCCommonViewController *)window.contentViewController;
            for (ASCTabView * tab in controller.tabsControl.tabs) {
                if (NSCefView * cefView = [controller cefViewWithTab:tab]) {
                    if ([cefView.data hasChanges]) {
                        unsaved++;
                    }
                    
                    // Blockchain check
                    if ([cefView checkCloudCryptoNeedBuild]) {
                        self.waitingForTerminateApp = YES;
                        return NO;
                    } else {
                        if ([cefView isSaveLocked]) {
                            unsaved++;
                            [locked_uuids addObject:tab.uuid];
                        }
                    }
                }
            }
        } else
        if ([window isKindOfClass:[ASCEditorWindow class]]) {
            ASCEditorWindow *editor = (ASCEditorWindow *)window;
            NSCefView *cefView = (NSCefView *)editor.webView;
            if ([cefView.data hasChanges]) {
                unsaved++;
            }
            
            // Blockchain check
            if ([cefView checkCloudCryptoNeedBuild]) {
                self.waitingForTerminateApp = YES;
                return NO;
            } else {
                if ([cefView isSaveLocked]) {
                    unsaved++;
                    // [locked_uuids addObject:[NSString stringWithFormat:@"%ld", cefView.uuid]];
                }
            }
        }
    }
    
    if (unsaved > 0) {
        NSString * productName = [ASCHelper appName];
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"Review Changes...", nil)];
        [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
        [alert addButtonWithTitle:NSLocalizedString(@"Delete and Quit", nil)];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"You have %ld %@ documents with unconfirmed changes. Do you want to review these changes before quitting?", nil), (long)unsaved, productName]];
        [alert setInformativeText:NSLocalizedString(@"If you don't review your documents, all your changeses will be lost.", nil)];
        [alert setAlertStyle:NSAlertStyleInformational];
        
        NSInteger result = [alert runModal];
        if (result == NSAlertFirstButtonReturn) {
            // Review Changes...
            self.waitingForTerminateApp = YES;
            
            NSInteger tabs_count = 0;
            if (controller) {
                [controller.tabsWithChanges removeAllObjects];
                NSArray * tabs = [NSArray arrayWithArray:controller.tabsControl.tabs];
                tabs_count = tabs.count;
                if (tabs_count > 0) {
                    NSMutableArray<ASCTabView *> *tabsWithChanges = [NSMutableArray array];
                    for (ASCTabView * tab in tabs) {
                        NSCefView * cefView = [controller cefViewWithTab:tab];
                        if ([cefView.data hasChanges] || [locked_uuids containsObject:tab.uuid]) {
                            [tabsWithChanges addObject:tab];
                        } else {
                            [controller.tabsControl removeTab:tab selected:NO animated:NO];
                        }
                    }
                    
                    controller.tabsWithChanges = [NSMutableArray arrayWithArray:tabsWithChanges];
                    [controller safeCloseTabsWithChanges];
                    
                } else {
                    [controller.view.window performClose:nil];
                }
            }
            
            if (tabs_count == 0) {
                [self safeCloseEditorWindows];
            }
            
            return NO;
            
        } else
        if (result == NSAlertSecondButtonReturn) {
            // Cancel
            return NO;
            
        } else {
            // Delete and Quit
            self.waitingForTerminateApp = YES;
            
            if (controller) {
                NSArray * tabs = [NSArray arrayWithArray:controller.tabsControl.tabs];
                for (ASCTabView * tab in tabs) {
                    [controller.tabsControl removeTab:tab selected:NO animated:NO];
                }
                 
                [controller.tabView selectTabViewItemWithIdentifier:rootTabId];
            }
            
            NSMutableArray *controllers = [NSMutableArray arrayWithArray:self.editorWindowControllers];
            for (NSWindowController *controller in controllers) {
                [controller.window close];
            }
            
            return NO;
        }
    }
    return YES;
}

- (void)presentMainWindow {
    if (self.mainWindowController) {
        [self.mainWindowController.window makeKeyAndOrderFront:nil];
    } else {
        NSStoryboard *storyboard = [NSStoryboard storyboardWithName:StoryboardNameMain bundle:nil];
        ASCTitleWindowController *windowController = [storyboard instantiateControllerWithIdentifier:@"ASCTitleWindowControllerId"];
        self.mainWindowController = windowController;
        ASCTitleWindow *mainWindow = (ASCTitleWindow *)windowController.window;
        [mainWindow makeKeyAndOrderFront:nil];
    }
}

- (void)safeCloseEditorWindows {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSWindowController *controller = [self.editorWindowControllers firstObject];
        if (controller) {
            [controller.window performClose:nil];
        }
    });
}

- (BOOL)validateMenuItem:(NSMenuItem *)item {
#ifdef _MAS
    [self.updateMenuItem setHidden:YES];
    [self.eulaMenuItem setHidden:YES];
#endif
    NSWindow *keyWindow = [NSApp keyWindow];
    ASCTabView * tab = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsCurrentTab];
    BOOL hasKeyWindow = (keyWindow != nil && ([keyWindow isKindOfClass:[ASCTitleWindow class]] || [keyWindow isKindOfClass:[ASCEditorWindow class]]));
    BOOL keyWindowSupportsDocumentOperations = hasKeyWindow && ([keyWindow isKindOfClass:[ASCEditorWindow class]] || tab != nil);
    NSString * productName = [ASCHelper appName];
    
    if ([item action] == @selector(onMenuAbout:)) {
        [item setTitle:[NSString stringWithFormat:NSLocalizedString(@"About %@", nil), productName]];
        return YES;
    } else if ([item action] == @selector(onMenuHide:)) {
        [item setTitle:[NSString stringWithFormat:NSLocalizedString(@"Hide %@", nil), productName]];
        return YES;
    } else if ([item action] == @selector(onMenuQuit:)) {
        [item setTitle:[NSString stringWithFormat:NSLocalizedString(@"Quit %@", nil), productName]];
        return YES;
    } else if ([item action] == @selector(onMenuNew:)) {
        return hasKeyWindow;
    } else if ([item action] == @selector(onMenuOpen:)) {
        return hasKeyWindow;
    } else if ([item action] == @selector(onMenuSave:)) {
        return keyWindowSupportsDocumentOperations;
    } else if ([item action] == @selector(onMenuSaveAs:)) {
        return keyWindowSupportsDocumentOperations;
    } else if ([item action] == @selector(onMenuPrint:)) {
        return keyWindowSupportsDocumentOperations;
    } else if ([item action] == @selector(onShowHelp:)) {
        [item setTitle:[NSString stringWithFormat:NSLocalizedString(@"%@ Help", nil), productName]];
        return YES;
    } else if ([item action] == @selector(onMenuAcknowledgments:)) {
        return YES;
    } else if ([item action] == @selector(onMenuEULA:)) {
        return YES;
    } else if ([item action] == @selector(onPreferences:)) {
        return YES;
    }
    
    return [super validateMenuItem:item];
}

#pragma mark -
#pragma mark Menu

- (IBAction)onShowHelp:(NSMenuItem *)sender {
    NSString * langCode = [[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString];
    NSString * helpUrl  = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @""];

    id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];

    if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
        langCode = [externalDelegate onAppPreferredLanguage];
    }

    if ([@"ru" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @"ru/"];
    } else if ([@"de" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @"de/"];
    } else if ([@"fr" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @"fr/"];
    } else if ([@"es" isEqualToString:langCode]) {
        helpUrl = [NSString stringWithFormat:[ASCConstants appInfo:kHelpUrl], @"es/"];
    }
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:helpUrl]];
}

- (IBAction)onMenuNew:(NSMenuItem *)sender {
    if (100 == sender.tag) {
        return;
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                        object:nil
                                                      userInfo:@{
                                                                 @"action"  : @(ASCTabActionCreateLocalFile),
                                                                 @"type"    : @(sender.tag),
                                                                 @"active"  : @(YES)
                                                                 }];
}

- (IBAction)onMenuOpen:(NSMenuItem *)sender {
    NSString * directiry = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsLastOpenDirectory];
    
    if (!directiry || directiry.length < 1) {
        directiry = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameOpenLocalFile
                                                        object:nil
                                                      userInfo:@{
                                                                 @"directory": directiry
                                                                 }];
}

- (IBAction)onMenuSave:(NSMenuItem *)sender {
    NSCefView * cefView = [self cefViewFromKeyWindow];
    if (cefView) {
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE);
        [cefView apply:pEvent];
    }
}

- (IBAction)onMenuSaveAs:(NSMenuItem *)sender {
    NSCefView * cefView = [self cefViewFromKeyWindow];
    if (cefView) {
        NSEditorApi::CAscExecCommandJS * pData = new NSEditorApi::CAscExecCommandJS;
        pData->put_Command(L"file:saveas");
        pData->put_FrameName(L"frameEditor");

        NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
        pEvent->m_pData = pData;
        
        [cefView apply:pEvent];
    }

}

- (IBAction)onMenuPrint:(NSMenuItem *)sender {
    NSCefView * cefView = [self cefViewFromKeyWindow];
    if (cefView) {
        NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
        pCommand->put_FrameName(L"frameEditor");
        pCommand->put_Command(L"file:print");

        NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
        pEvent->m_pData = pCommand;

        [cefView apply:pEvent];
    }
}

- (IBAction)onMenuAcknowledgments:(NSMenuItem *)sender {
    [self presentMainWindow];
    NSWindow * mainWindow = [self getMainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    [controller openAcknowledgments];
}

- (IBAction)onMenuEULA:(NSMenuItem *)sender {
    [self presentMainWindow];
    NSWindow * mainWindow = [self getMainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    [controller openEULA];
}

- (IBAction)onMenuAbout:(NSMenuItem *)sender {
//    [NSApp orderFrontStandardAboutPanel:sender];
    [self presentMainWindow];
    NSWindow * mainWindow = [self getMainWindow];
    
    if (mainWindow) {
        ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
        
        NSWindowController * windowController = [controller.storyboard instantiateControllerWithIdentifier:@"ASCAboutWindowControllerId"];
        [NSApp runModalForWindow:windowController.window];
    }
}

- (IBAction)onPreferences:(NSMenuItem *)sender {
    [self presentMainWindow];
    NSWindow * mainWindow = [self getMainWindow];
    ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
    [controller openPreferences];
}

- (IBAction)onMenuHide:(NSMenuItem *)sender {
    [NSApp hide:sender];
}

- (IBAction)onMenuQuit:(NSMenuItem *)sender {
    [NSApp terminate:sender];
}

- (NSWindow *)getMainWindow {
    return self.mainWindowController ? self.mainWindowController.window : nil;
}

- (NSWindow *)effectiveKeyWindow {
    NSWindow *keyWindow = [NSApp keyWindow];
    if (keyWindow && ([keyWindow isKindOfClass:[ASCTitleWindow class]] || [keyWindow isKindOfClass:[ASCEditorWindow class]])) {
        return keyWindow;
    }
    for (NSWindow *window in [NSApp orderedWindows]) {
        if ([window isKindOfClass:[ASCTitleWindow class]] || [window isKindOfClass:[ASCEditorWindow class]]) {
            return window;
        }
    }
    return nil;
}

- (NSCefView *)cefViewFromKeyWindow {
    NSWindow *keyWindow = [NSApp keyWindow];
    if (!keyWindow) {
        return nil;
    }
    
    if ([keyWindow isKindOfClass:[ASCTitleWindow class]]) {
        ASCTabView *tab = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsCurrentTab];
        if (tab) {
            ASCCommonViewController *controller = (ASCCommonViewController *)keyWindow.contentViewController;
            return [controller cefViewWithTab:tab];
        }
    } else
    if ([keyWindow isKindOfClass:[ASCEditorWindow class]]) {
        ASCEditorWindow *editor = (ASCEditorWindow *)keyWindow;
        return (NSCefView *)editor.webView;
    }
    return nil;
}

- (void)saveLocalFileWithParams:(NSDictionary *)params {
    if (params) {
        NSString * path         = params[@"path"];
        NSString * viewId       = params[@"viewId"];
        NSArray * formats       = params[@"supportedFormats"];
        
        //        __block NSInteger fileType = [params[@"fileType"] intValue];
        
        __block ASCSavePanelWithFormatController * saveController = [ASCSavePanelWithFormatController new];
        
        NSSavePanel * savePanel = [saveController savePanel];
        
        saveController.filters = formats;
        saveController.original = params[@"original"];
        //        saveController.filterType = fileType;
        
        if (!path || path.length < 1) {
            path = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }
        
        BOOL isDir;
        if (![[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir]) {
            NSString * savedPath = [[NSUserDefaults standardUserDefaults] objectForKey:ASCUserLastSavePath];
            
            if (savedPath && savedPath.length > 0) {
                path = [savedPath stringByAppendingPathComponent:path];
            } else {
                path = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject] stringByAppendingPathComponent:path];
            }
        }
        
        savePanel.directoryURL = [NSURL fileURLWithPath:[path stringByDeletingLastPathComponent]];
        savePanel.canCreateDirectories = YES;
        savePanel.nameFieldStringValue = [[path lastPathComponent] stringByDeletingPathExtension];
        
        [savePanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [savePanel orderOut:self];
            
            NSEditorApi::CAscLocalSaveFileDialog * saveData = new NSEditorApi::CAscLocalSaveFileDialog();
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            
            if (result == NSFileHandlingPanelOKButton) {
                [[NSUserDefaults standardUserDefaults] setObject:[[savePanel directoryURL] path] forKey:ASCUserLastSavePath];
                [[NSUserDefaults standardUserDefaults] synchronize];
                
                NSString * path = [NSString stringWithFormat:@"%@", [[savePanel URL] path]];
                
                saveData->put_Path([path stdwstring]);
                saveData->put_Id([viewId intValue]);
                saveData->put_FileType((int)[saveController filterType]);
            } else {
                saveData->put_Id([viewId intValue]);
                saveData->put_Path(L"");
            }
            
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH);
            pEvent->m_pData = saveData;
            
            appManager->Apply(pEvent);
        }];
    }
}

- (BOOL)canOpenFile:(NSString *)path tab:(ASCTabView *)tab {
    BOOL canOpen = NO;
    
    if (path) {
        
        NSURL * urlFile = [NSURL URLWithString:[path stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLQueryAllowedCharacterSet]]];
        
        if (urlFile && [urlFile host]) {
            canOpen = YES;
        } else {
            int fileFormatType = CCefViewEditor::GetFileFormat([path stdwstring]);
            canOpen = (0 != fileFormatType);
        }
    }
    
    if (!canOpen) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"OK", nil)];
        [alert setMessageText:NSLocalizedString(@"File can not be open.", nil)];
        [alert setInformativeText:[NSString stringWithFormat:NSLocalizedString(@"File \"%@\" can not be open or not exist.", nil), path]];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert beginSheetModalForWindow:[NSApp mainWindow]  completionHandler:^(NSModalResponse returnCode) {
            if (tab) {
                //[self.tabsControl removeTab:tab animated:NO];
            }
        }];
    }
    
    return canOpen;
}

#pragma mark -
#pragma mark App Appearance

- (void)updateAppAppearance {
    if (@available(macOS 10.14, *)) {
        NSString * theme = [ASCThemesController currentThemeId];
        if ([theme isEqualToString:uiThemeSystem]) {
            [NSApp setAppearance:nil];
        } else
        if ([ASCThemesController isCurrentThemeDark]) {
            [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];
        } else {
            [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameAqua]];
        }
    }
}

#pragma mark -
#pragma mark Notification handlers

- (void)onSystemThemeChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        [self updateAppAppearance];
        
        NSDictionary * info = (NSDictionary *)notification.userInfo;
        NSString * mode = info[@"mode"];
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        NSMutableDictionary * json = [[NSMutableDictionary alloc] initWithDictionary: @{@"theme": @{@"system": mode}}];
        std::wstring params = [[json jsonString] stdwstring];
        
        NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
        pCommand->put_Command(L"renderervars:changed");
        pCommand->put_Param(params);
        
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
        pEvent->m_pData = pCommand;
        appManager->SetEventToAllMainWindows(pEvent);
        
        for (int uuid : appManager->GetViewsId()) {
            CCefView * cef = appManager->GetViewById((int)uuid);
            if (cef && cef->GetType() == cvwtEditor) {
                pCommand = new NSEditorApi::CAscExecCommandJS;
                pCommand->put_FrameName(L"frameEditor");
                pCommand->put_Command(L"renderervars:changed");
                pCommand->put_Param(params);
                
                pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                pEvent->m_pData = pCommand;
                cef->Apply(pEvent);
            }
        }
        
        [[ASCEditorJSVariables instance] setVariable:@"theme" withObject:@{@"id": [ASCThemesController currentThemeId],
                                                                         @"type": [ASCThemesController isCurrentThemeDark] ? @"dark" : @"light",
                                                                       @"system": mode}];
        [[ASCEditorJSVariables instance] apply];
        
        json = [[NSMutableDictionary alloc] initWithDictionary: @{@"theme": [ASCThemesController actualThemeId]}];
        appManager->UpdatePlugins([[json jsonString] stdwstring]);
    }
}

- (void)onUIThemeChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        [self updateAppAppearance];
        
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        std::wstring wtheme = [params[@"uitheme"] stdwstring];
        NSString * theme = params[@"uitheme"];
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
        pCommand->put_Command(L"uitheme:changed");
        pCommand->put_Param(wtheme);
        
        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
        pEvent->m_pData = pCommand;
        appManager->SetEventToAllMainWindows(pEvent);
        
        NSMutableDictionary * json = [[NSMutableDictionary alloc] initWithDictionary: @{@"theme": theme}];
        appManager->UpdatePlugins([[json jsonString] stdwstring]);
       
        for (int uuid : appManager->GetViewsId()) {
            CCefView * cef = appManager->GetViewById((int)uuid);
            if (cef && cef->GetType() == cvwtEditor) {
                pCommand = new NSEditorApi::CAscExecCommandJS;
                pCommand->put_FrameName(L"frameEditor");
                pCommand->put_Command(L"uitheme:changed");
                pCommand->put_Param(wtheme);

                pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                pEvent->m_pData = pCommand;
                cef->Apply(pEvent);
            }
        }
        
        [[ASCEditorJSVariables instance] setParameter:@"uitheme" withString:theme];
        [[ASCEditorJSVariables instance] applyParameters];
    }
}

#pragma mark -
#pragma mark CEF events handlers

- (void)onCEFCreateTab:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSMutableDictionary * params = [notification.userInfo mutableCopy];
        
        if ([params[@"action"] isEqualToNumber:@(ASCTabActionCreateLocalFileFromTemplate)]) {
            if ( [params objectForKey:@"path"] or [params objectForKey:@"id"] ) {}
            else {
                NSOpenPanel * openPanel = [NSOpenPanel openPanel];
                NSMutableArray * filter = [NSMutableArray array];
                
                if ( [params[@"type"] isEqualToNumber:@((int)AscEditorType::etPresentation)] ) {
                    [filter addObjectsFromArray:@[@"potx", @"otp"]];
                } else if ( [params[@"type"] isEqualToNumber:@((int)AscEditorType::etSpreadsheet)] ) {
                    [filter addObjectsFromArray:@[@"xltx", @"xltm", @"ots"]];
                } else {
                    [filter addObjectsFromArray:@[@"dotx", @"ott"]];
                }
                
                openPanel.canChooseDirectories = NO;
                openPanel.allowsMultipleSelection = NO;
                openPanel.canChooseFiles = YES;
                openPanel.allowedFileTypes = filter;
                
                if ([openPanel runModal] == NSModalResponseOK) {
                    [params setValue:[[openPanel URL] path] forKey:@"path"];
                } else return;
            }
        } else
        if ([params[@"action"] isEqualToNumber:@(ASCTabActionOpenLocalRecentFile)] ||
            [params[@"action"] isEqualToNumber:@(ASCTabActionOpenLocalFile)])
        {
            if ( ![self canOpenFile:params[@"path"] tab:nil] ) {
                return;
            }
        }
        
        for (NSWindow *window in [NSApp windows]) {
            if ([window isKindOfClass:[ASCTitleWindow class]]) {
                ASCCommonViewController *common = (ASCCommonViewController *)window.contentViewController;
                ASCTabView * existTab = [common tabWithParam:@"url" value:params[@"url"]];
                if (!existTab) {
                    existTab = [common tabWithParam:@"path" value:params[@"path"]];
                }
                if (existTab) {
                    [common.tabsControl selectTab:existTab];
                    return;
                }
                
            } else
            if ([window isKindOfClass:[ASCEditorWindow class]]) {
                ASCEditorWindow *editor = (ASCEditorWindow *)window;
                NSCefView *cefView = (NSCefView *)editor.webView;
                if (cefView) {
                    if ([cefView.data.url isEqualToString:params[@"url"]] || [cefView.data.path isEqualToString:params[@"path"]]) {
                        [editor makeKeyAndOrderFront:nil];
                        return;
                    }
                }
            }
        }
        
        ASCTabActionType action = (ASCTabActionType)[params[@"action"] intValue];
        NSString *title = [NSString stringWithFormat:@"%@...", NSLocalizedString(@"Opening", nil)];
        
        if ([self preferOpenEditorWindow] && action != ASCTabActionOpenPortal) {
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            NSCefView * cefView = [[NSCefView alloc] initWithFrame:CGRectZero];
            
            NSCefData *cefData = [[NSCefData alloc] initWith:title viewType:cvwtEditor];
            
            cefView.data = cefData;
            
            [cefView create:appManager withType:cvwtEditor];
            [cefView setBackgroundColor:[ASCThemesController currentThemeColor:windowBackgroundColor]];
            
            NSDictionary *widgetInfo = @{@"widgetType": @"window", @"captionHeight": TOOLBTN_HEIGHT};
            [cefView setParentWidgetInfoWithJson:[widgetInfo jsonString]];
            
            ASCTabViewType type = ASCTabViewTypeOpening;
            switch (action) {
                case ASCTabActionOpenUrl: {
                    [cefView loadWithUrl:params[@"url"]];
                    
                    if (params[@"title"] && [params[@"title"] length] > 0) {
                        title = params[@"title"];
                    }
                    
                    break;
                }
                
                case ASCTabActionCreateLocalFileFromTemplate:
                case ASCTabActionCreateLocalFile: {
                    AscEditorType docType = AscEditorType::etDocument;
                    if ( [params[@"type"] isKindOfClass:[NSString class]] ) {
                        NSString * param = params[@"type"];
                        if ([param isEqualToString:@"cell"]) docType = AscEditorType::etSpreadsheet;
                        else if ([param isEqualToString:@"slide"]) docType = AscEditorType::etPresentation;
                        else if ([param isEqualToString:@"form"]) docType = AscEditorType::etDocumentMasterForm;
//                        else /*if ([param isEqualToString:@"word"])*/ docType = AscEditorType::etDocument;
                    } else docType = (AscEditorType)[params[@"type"] intValue];
                    
                    NSString * docName = NSLocalizedString(@"Untitled", nil);
                    
                    switch (docType) {
                        case AscEditorType::etDocument:
                            docName = [NSString stringWithFormat:NSLocalizedString(@"Document %ld.docx", nil), ++self.documentNameCounter];
                            break;
                        case AscEditorType::etSpreadsheet:
                            docName = [NSString stringWithFormat:NSLocalizedString(@"Spreadsheet %ld.xlsx", nil), ++self.spreadsheetNameCounter];
                            break;
                        case AscEditorType::etPresentation:
                            docName = [NSString stringWithFormat:NSLocalizedString(@"Presentation %ld.pptx", nil), ++self.presentationNameCounter];
                            break;
                        case AscEditorType::etDocumentMasterOForm:
                        case AscEditorType::etDocumentMasterForm:
                            docName = [NSString stringWithFormat:NSLocalizedString(@"Document %ld.pdf", nil), ++self.pdfNameCounter];
                            break;
                        default: break;
                    }
                    
                    if (action == ASCTabActionCreateLocalFile ) {
                        [cefView createFileWithName:docName type:docType];
                    } else {
                        [cefView createFileWithNameFromTemplate:docName tplpath:params[@"path"]];
                    }
                    
                    break;
                }
                
                case ASCTabActionOpenLocalFile: {
                    NSString * filePath = params[@"path"];
                    
                    int fileFormatType = CCefViewEditor::GetFileFormat([filePath stdwstring]);
                    [cefView openFileWithName:filePath type:fileFormatType];
                    
                    [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                             action:@"Open local file"
                                                                              label:nil
                                                                              value:nil];
                    
                    break;
                }
                case ASCTabActionOpenLocalRecoverFile: {
                    NSInteger docId = [params[@"fileId"] intValue];
                    [cefView openRecoverFileWithId:docId];
                    
                    [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                             action:@"Open local recover file"
                                                                              label:nil
                                                                              value:nil];
                    
                    break;
                }
                case ASCTabActionOpenLocalRecentFile: {
                    NSInteger docId = [params[@"fileId"] intValue];
                    
                    if ( !(docId < 0) )
                        [cefView openRecentFileWithId:docId];
                    else {
                        NSString * filePath = params[@"path"];
                        if ( filePath && filePath.length )
                            [cefView loadWithUrl:filePath];
                    }
                    
                    [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                             action:@"Open local file"
                                                                              label:nil
                                                                              value:nil];
                    
                    break;
                }
                
                default:
                    break;
            }
            
            ASCEditorWindowController *windowController = nil;
            if (self.mainWindowController && self.mainWindowController.window) {
                ASCTitleWindowController *mainWindowController = (ASCTitleWindowController *)self.mainWindowController;
                NSRect mainFrame = [mainWindowController normalFrame];
                mainFrame.origin.x += 30;
                mainFrame.origin.y -= 30;
                windowController = [ASCEditorWindowController initWithFrame:mainFrame];
                if ( [mainWindowController.window isZoomed] ) {
                    [windowController.window setIsZoomed:YES];
                }
            } else {
                windowController = [ASCEditorWindowController initWithDefaultFrame];
            }
            [self.editorWindowControllers addObject:windowController];
            
            ASCEditorWindow *editorWindow = (ASCEditorWindow *)windowController.window;
            [editorWindow setTitleVisibility:NSWindowTitleHidden];
            [editorWindow setTitle:title];
            
            NSViewController *contentViewController = windowController.contentViewController;
            if (contentViewController && contentViewController.view) {
                editorWindow.webView = cefView;
                [contentViewController.view addSubview:cefView];
                [cefView autoPinEdgesToSuperviewEdges];
                NSLog(@"Tab detached: WebView moved to new window");
            }
            
            [editorWindow makeKeyAndOrderFront:nil];
            
            NSDictionary *windowFeatures = @{@"skiptoparea": TOOLBTN_HEIGHT, @"singlewindow": @YES};
            [cefView sendCommand:@"window:features" withParam:[windowFeatures jsonString]];
            
        } else {
            [self presentMainWindow];
            ASCTitleWindowController *controller = (ASCTitleWindowController *)self.mainWindowController;
            ASCCommonViewController *common = (ASCCommonViewController *)controller.contentViewController;
            
            ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
            tab.title       = [NSString stringWithFormat:@"%@...", NSLocalizedString(@"Opening", nil)];
            tab.type        = ASCTabViewTypeOpening;
            tab.params      = [params mutableCopy];
            
            if ([params[@"action"] isEqualToNumber:@(ASCTabActionCreateLocalFile)]) {
                // Prevent add tab if necessary
            }
            
            if (params[@"external"]) {
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.9 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                    [common.tabsControl addTab:tab selected:[params[@"active"] boolValue]];
                });
            } else {
                [common.tabsControl addTab:tab selected:[params[@"active"] boolValue]];
            }
        }
    }
}

- (void)onCEFStartSave:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * fileName = notification.userInfo[@"fileName"];
        NSNumber * idx      = notification.userInfo[@"idx"];
        
        NSSavePanel * savePanel = [NSSavePanel savePanel];
        //        [savePanel setDirectoryURL:[NSURL URLWithString:[NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory, NSUserDomainMask, YES) firstObject]]];
        if (fileName && fileName.length > 0) {
            [savePanel setAllowedFileTypes:@[fileName.pathExtension]];
            [savePanel setNameFieldStringValue:[fileName lastPathComponent]];
        }
        
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        [savePanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [savePanel orderOut:self];
            
            if (result == NSFileHandlingPanelOKButton) {
                appManager->EndSaveDialog([[[savePanel URL] path] stdwstring], [idx unsignedIntValue]);
            } else {
                appManager->EndSaveDialog(L"", [idx unsignedIntValue]);
            }
        }];
    }
}

- (void)onCEFSaveLocalFile:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
        
        for (NSWindow *window in [NSApp windows]) {
            if ([window isKindOfClass:[ASCTitleWindow class]]) {
                ASCCommonViewController * controller = (ASCCommonViewController *)window.contentViewController;
                ASCTabView * tab = [controller.tabsControl tabWithUUID:viewId];
                if (tab) {
                    [tab.params addEntriesFromDictionary:params];
                    [controller.tabsControl selectTab:tab];
                    [self saveLocalFileWithParams:tab.params];
                    
                    [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                             action:@"Save local file"
                                                                              label:nil
                                                                              value:nil];
                    break;
                }
                
            } else
            if ([window isKindOfClass:[ASCEditorWindow class]]) {
                ASCEditorWindow *editor = (ASCEditorWindow *)window;
                ASCEditorWindowController *controller = (ASCEditorWindowController *)editor.windowController;
                if (controller && [controller holdView:viewId]) {
                    [controller.params addEntriesFromDictionary:params];
                    [self saveLocalFileWithParams:params];
                    
                    [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                             action:@"Save local file"
                                                                              label:nil
                                                                              value:nil];
                    break;
                }
            }
        }
    }
}

- (void)onCEFOnOpenLocalFile:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * directory = notification.userInfo[@"directory"];
        
        if (!directory || directory.length < 1) {
            directory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }
        
        NSOpenPanel * openPanel = [NSOpenPanel openPanel];
        NSMutableArray * filter = [NSMutableArray array];
        [filter addObjectsFromArray:[ASCConstants documents]];
        [filter addObjectsFromArray:[ASCConstants spreadsheets]];
        [filter addObjectsFromArray:[ASCConstants presentations]];
        [filter addObjectsFromArray:[ASCConstants draws]];
        
        openPanel.canChooseDirectories = NO;
        openPanel.allowsMultipleSelection = NO;
        openPanel.canChooseFiles = YES;
        openPanel.allowedFileTypes = filter;
        openPanel.directoryURL = [NSURL fileURLWithPath:directory];
        
        [openPanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [openPanel orderOut:self];
            
            if (result == NSFileHandlingPanelOKButton) {
                [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                    object:nil
                                                                  userInfo:@{
                    @"action"  : @(ASCTabActionOpenLocalFile),
                    @"path"    : [[openPanel URL] path],
                    @"active"  : @(YES)
                }];
            }
        }];
    }
}

- (void)onCEFEditorEvent:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        NSDictionary * data = json[@"data"];
        
        if (viewId && data) {
            NSString * action = data[@"action"];
            
            if ( action ) {
                if ( [action isEqualToString:@"file:close"] ) {
                    for (NSWindow *window in [NSApp windows]) {
                        if ([window isKindOfClass:[ASCTitleWindow class]]) {
                            ASCCommonViewController * controller = (ASCCommonViewController *)window.contentViewController;
                            ASCTabView * tab = [controller.tabsControl tabWithUUID:viewId];
                            if (tab) {
                                [controller.tabsControl removeTab:tab selected:NO animated:NO];
                                break;
                            }
                            
                        } else
                        if ([window isKindOfClass:[ASCEditorWindow class]]) {
                            ASCEditorWindow *editor = (ASCEditorWindow *)window;
                            ASCEditorWindowController *controller = (ASCEditorWindowController *)editor.windowController;
                            if (controller && [controller holdView:viewId]) {
                                [editor close];
                                break;
                            }
                        }
                    }
                    
                } else
                if ( [action isEqualToString:@"file:open"] ){
                    NSNotification * notification = [NSNotification notificationWithName: CEFEventNameOpenLocalFile
                                                                                  object: nil
                                                                                userInfo: @{@"directory":@""}];
                    [self onCEFOnOpenLocalFile: notification];
                }
            }
        }
    }
}

- (void)onCEFOpenLocalImage:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * directory = notification.userInfo[@"path"];
        NSInteger fileId = [notification.userInfo[@"fileId"] intValue];
        
        if (!directory || directory.length < 1) {
            directory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }
        
        NSOpenPanel * openPanel = [NSOpenPanel openPanel];
        
        openPanel.canChooseDirectories = NO;
        openPanel.allowsMultipleSelection = NO;
        openPanel.canChooseFiles = YES;
        openPanel.allowedFileTypes = [ASCConstants images];
        openPanel.directoryURL = [NSURL fileURLWithPath:directory];
        
        [openPanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [openPanel orderOut:self];
            
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            
            NSEditorApi::CAscLocalOpenFileDialog * imageInfo = new NSEditorApi::CAscLocalOpenFileDialog();
            imageInfo->put_Id((int)fileId);
            
            if (result == NSFileHandlingPanelOKButton) {
                imageInfo->put_Path([[[openPanel URL] path] stdwstring]);
            } else {
                imageInfo->put_Path(L"");
            }
            
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE);
            pEvent->m_pData = imageInfo;
            
            appManager->Apply(pEvent);
        }];
    }
}

- (void)onCEFOpenFileDialog:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * directory = notification.userInfo[@"path"];
        NSString * fileTypes = notification.userInfo[@"filter"];
        NSInteger fileId = [notification.userInfo[@"fileId"] intValue];
        BOOL isMulti = [notification.userInfo[@"isMulti"] boolValue];
        
        if (!directory || directory.length < 1) {
            directory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }
        
        NSArray * allowedFileTypes = @[];
        
        if ([fileTypes length] == 0) {
            NSMutableArray * filter = [NSMutableArray array];
            [filter addObjectsFromArray:[ASCConstants documents]];
            [filter addObjectsFromArray:[ASCConstants spreadsheets]];
            [filter addObjectsFromArray:[ASCConstants presentations]];
            [filter addObjectsFromArray:[ASCConstants draws]];
            [filter addObjectsFromArray:[ASCConstants csvtxt]];
            
            allowedFileTypes = filter;
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterImage]) {
            allowedFileTypes = [ASCConstants images];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterAudio]) {
            allowedFileTypes = [ASCConstants audios];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterVideo]) {
            allowedFileTypes = [ASCConstants videos];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterPlugin]) {
            allowedFileTypes = [ASCConstants plugins];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterDocument]) {
            allowedFileTypes = [ASCConstants documents];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterSpreadsheet]) {
            allowedFileTypes = [ASCConstants spreadsheets];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterPresentation]) {
            allowedFileTypes = [ASCConstants presentations];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterCsvTxt]) {
            allowedFileTypes = [ASCConstants csvtxt];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterCrypto]) {
            allowedFileTypes = [ASCConstants cancryptformats];
        } else if ([fileTypes isEqualToString:CEFOpenFileFilterXML]) {
            allowedFileTypes = [ASCConstants xmldata];
        } else if ([fileTypes isEqualToString:@"any"] || [fileTypes isEqualToString:@"*.*"]) {
            //            allowedFileTypes = @[@"*.*"];
        } else {
            // filters come in view "*.docx *.pptx *.xlsx"
            NSError *error = nil;
            NSRegularExpression * regex = [NSRegularExpression regularExpressionWithPattern:@"[\\(\\)\\*\\.]" options:NSRegularExpressionCaseInsensitive error:&error];
            NSString * filters = [regex stringByReplacingMatchesInString:fileTypes options:0 range:NSMakeRange(0, [fileTypes length]) withTemplate:@""];
            allowedFileTypes = [filters componentsSeparatedByString:@" "];
        }
        
        NSOpenPanel * openPanel = [NSOpenPanel openPanel];
        
        openPanel.canChooseDirectories = NO;
        openPanel.allowsMultipleSelection = NO;
        openPanel.canChooseFiles = YES;
        openPanel.directoryURL = [NSURL fileURLWithPath:directory];
        
        if (allowedFileTypes.count > 0) {
            openPanel.allowedFileTypes = allowedFileTypes;
        }
        
        [openPanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [openPanel orderOut:self];
            
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            
            NSEditorApi::CAscLocalOpenFileDialog * imageInfo = new NSEditorApi::CAscLocalOpenFileDialog();
            imageInfo->put_Id((int)fileId);
            imageInfo->put_Filter([fileTypes stdwstring]);
            
            if (result == NSFileHandlingPanelOKButton) {
                imageInfo->put_Path([[[openPanel URL] path] stdwstring]);
            } else {
                imageInfo->put_Path(L"");
            }
            
            if (isMulti) {
                imageInfo->put_IsMultiselect(true);
            }
            
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG);
            pEvent->m_pData = imageInfo;
            
            appManager->Apply(pEvent);
        }];
    }
}

- (void)onCEFFileInFinder:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * info = notification.userInfo[@"info"];
        if (NSDictionary * json = [info dictionary]) {
            NSURL * fileUrl = [NSURL fileURLWithPath:json[@"path"]];
            [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:@[fileUrl]];
        }
    }
}

- (void)onCEFCertificatePreview:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * text = json[@"text"];
        NSString * path = json[@"path"];
        
        if (path && path.length > 0) {
            ASCCertificateQLPreviewController * controller = [ASCCertificateQLPreviewController new];
            [controller previewBy:[NSURL fileURLWithPath:path]];
        } else
        if (text && text.length > 0) {
            NSWindow * window = [self effectiveKeyWindow];
            NSViewController * controller = nil;
            if (window && (controller = window.contentViewController) != nil) {
                ASCCertificatePreviewController * previewController = [[ASCCertificatePreviewController alloc] init:controller];
                [previewController presentTextInfo:text];
            }
        }
    }
}

- (void)onCEFEditorDocumentReady:(NSNotification *)notification {
    //
    if (notification && notification.userInfo) {
        NSArray * printers = [NSPrinter printerNames];
        
        if ([printers count] != 0) {
            NSMutableArray * arr = [NSMutableArray array];
            
            for (NSString * printerName in printers) {
                [arr addObject:@{@"name": printerName}];
            }
            
            id info = notification.userInfo;
            if (NSString * viewId = info[@"viewId"]) {
                CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                
                int cefViewId = [viewId intValue];
                CCefView * cef = appManager->GetViewById(cefViewId);
                if (cef && cef->GetType() == cvwtEditor) {
                    NSString * def_printer_name = arr[0][@"name"];
                    NSMutableDictionary * json = [[NSMutableDictionary alloc] initWithDictionary:
                                                  @{
                        @"current_printer": def_printer_name,
                        @"printers": arr
                    }];
                    
                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                    pCommand->put_FrameName(L"frameEditor");
                    pCommand->put_Command(L"printer:config");
                    pCommand->put_Param([[json jsonString] stdwstring]);
                    
                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                    pEvent->m_pData = pCommand;
                    
                    cef->Apply(pEvent);
                }
            }
        }
    }
}

- (void)onCEFKeyDown:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        
        NSValue * eventData = params[@"data"];
        
        //NSLog(@"ui oncefkeydown");
        if (eventData) {
            NSEditorApi::CAscKeyboardDown * pData = (NSEditorApi::CAscKeyboardDown *)[eventData pointerValue];
            
            int keyCode = pData->get_KeyCode();
            if ( keyCode == 112 /*kVK_F1*/ && pData->get_IsShift() && pData->get_IsCtrl() ) {
                NSOpenPanel * openPanel = [NSOpenPanel openPanel];
                
                openPanel.canChooseDirectories = YES;
                openPanel.allowsMultipleSelection = NO;
                openPanel.canChooseFiles = NO;
                openPanel.allowedFileTypes = [ASCConstants images];
                //                openPanel.directoryURL = [NSURL fileURLWithPath:directory];
                
                [openPanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
                    [openPanel orderOut:self];
                    
                    if (result == NSFileHandlingPanelOKButton) {
                        NSString * pathToHelp = [[[openPanel directoryURL] path] stringByAppendingString: @"/apps"];
                        NSString * pathContents = [pathToHelp stringByAppendingString:@"/documenteditor/main/resources/help/en/Contents.json"];
                        
                        NSAlert * alert = [[NSAlert alloc] init];
                        if ( [[NSFileManager defaultManager] fileExistsAtPath:pathContents] ) {
                            [[NSUserDefaults standardUserDefaults] setValue:pathToHelp forKey:@"helpUrl"];
                            [[ASCEditorJSVariables instance] setVariable:@"helpUrl" withString:pathToHelp];
                            [[ASCEditorJSVariables instance] apply];
                            
                            [alert setMessageText:@"Successfully"];
                        } else {
                            [alert setMessageText:@"Failed"];
                        }
                        [alert runModal];
                    }
                }];
            } else
            if ( keyCode == 9 ) { // Tab
                if ( pData->get_IsCtrl() ) {
                    NSWindow *keyWindow = [NSApp keyWindow];
                    if (keyWindow && [keyWindow isKindOfClass:[ASCTitleWindow class]]) {
                        ASCCommonViewController * controller = (ASCCommonViewController *)keyWindow.contentViewController;
                        if ( pData->get_IsShift() ) {
                            [controller.tabsControl selectPreviouseTab];
                        } else {
                            [controller.tabsControl selectNextTab];
                        }
                    }
                }
            } else
            if ( keyCode == 87 ) { // W
                // Handled by AppKit
//                if ( pData->get_IsCommandMac() ) {
//                    ASCTabView * tab = [self.tabsControl selectedTab];
//                    if ( tab and [self tabs:self.tabsControl willRemovedTab:tab] ) {
//                        [self.tabsControl removeTab:tab animated:NO];
//                    }
//                }
            } else
            if ( keyCode == 81 ) { // Q
                // Handled by AppKit
//                if ( pData->get_IsCommandMac() ) {
//                    [NSApp terminate:self];
//                }
            }
        }
    }
}

- (void)onCEFSaveBeforeSign:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        if (viewId) {
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            
            int cefViewId = [viewId intValue];
            CCefView * cef = appManager->GetViewById(cefViewId);
            if (cef && cef->GetType() == cvwtEditor) {
                NSAlert *alert = [NSAlert new];
                [alert addButtonWithTitle:NSLocalizedString(@"Save", nil)];
                [[alert addButtonWithTitle:NSLocalizedString(@"Don't Save", nil)] setKeyEquivalent:@"\e"];
                [alert setMessageText:NSLocalizedString(@"Before signing the document, it must be saved.", nil)];
                [alert setInformativeText:NSLocalizedString(@"Save the document?", nil)];
                [alert setAlertStyle:NSAlertStyleWarning];
                
                NSInteger returnCode = [alert runModalSheet];
                
                NSEditorApi::CAscEditorSaveQuestion * pEventData = new NSEditorApi::CAscEditorSaveQuestion();
                NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
                
                pEventData->put_Value(returnCode == NSAlertFirstButtonReturn);
                pEvent->m_pData = pEventData;
                
                cef->Apply(pEvent);
            }
        }
    }
}

- (void)printOperationDidRun:(NSPrintOperation *)printOperation success:(BOOL)success contextInfo:(void *)contextInfo {
    if (m_pContext) {
        m_pContext->EndPaint();
        
        m_pContext->Release();
        m_pContext = nullptr;
    }
}

- (void)onCEFOnBeforePrintEnd:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        //        NSNumber * viewId       = notification.userInfo[@"viewId"];
        //        NSNumber * pagesCount   = notification.userInfo[@"countPages"];
        //        NSNumber * pagesCount   = notification.userInfo[@"currentPage"];
        NSString * options      = notification.userInfo[@"options"];
        
        NSDictionary * nameLocales = [options dictionary];
        NSLog(@"options: %@", nameLocales);
        
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        // using synchronization to be sure that flag `ASCPrinterContext::isCurrentlyPrinting` is correctly handled
        static dispatch_queue_t printQueue = dispatch_queue_create(NULL, NULL);
        dispatch_sync(printQueue, ^{
            if (appManager && !ASCPrinterContext::isCurrentlyPrinting) {
                m_pContext = new ASCPrinterContext(appManager);
                //            m_pContext->BeginPaint([viewId intValue], [pagesCount intValue], self, @selector(printOperationDidRun:success:contextInfo:));
                m_pContext->BeginPaint(notification.userInfo, self, @selector(printOperationDidRun:success:contextInfo:));
            }
        });
    }
}

-(void)onRecoveryFiles:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * sfiles = params[@"files"];
        NSError * err = nil;
        NSArray * arrfiles = [NSJSONSerialization JSONObjectWithData:[sfiles dataUsingEncoding:NSUTF8StringEncoding]
                                                             options:0
                                                               error:&err];
        for (NSDictionary * f in arrfiles) {
            NSNotification * n = [NSNotification notificationWithName:CEFEventNameCreateTab
                                                               object:nil
                                                             userInfo:@{
                @"action"  : @(ASCTabActionOpenLocalRecoverFile),
                @"active"  : @(YES),
                @"fileId"  : f[@"id"],
                @"path"    : f[@"path"]
            }];
            [self onCEFCreateTab:n];
        }
    }
}

#pragma mark -
#pragma mark Tab Attachment Support

- (ASCTitleBarController *)titleBarController {
    ASCTitleWindowController *controller = (ASCTitleWindowController *)self.mainWindowController;
    return controller ? controller.titlebarController : nil;
}

- (void)pinWindowToTab:(ASCEditorWindow *)window atPoint:(NSPoint)screenPoint {
    ASCTitleBarController *titlebarController = [self titleBarController];
    if (titlebarController) {
        [titlebarController attachEditor:window.webView atScreenPoint:screenPoint];
        window.webView = nil;
        [window close];
    }
}

- (void)handleDropTimer {
    ASCTitleBarController *titlebarController = [self titleBarController];
    NSPoint currentCursor = [NSEvent mouseLocation];
    if (titlebarController && [titlebarController canPinTabAtPoint:currentCursor]) {
        if (NSEqualPoints(currentCursor, lastCursorPos)) {
            [self stopDropTimer];
            
            NSEventMask buttons = [NSEvent pressedMouseButtons];
            if (buttons & (1 << 0)) { // Left button pressed
                ASCEditorWindow *editorWindow = (ASCEditorWindow *)self.dropEditorWindow;
                [self pinWindowToTab:editorWindow atPoint:currentCursor];
            }
        } else {
            lastCursorPos = currentCursor;
        }
    } else {
        [self stopDropTimer];
    }
}

- (void)stopDropTimer {
    if (dropTimer && [dropTimer isValid]) {
        [dropTimer invalidate];
    }
    dropTimer = nil;
}

- (void)validateDrop:(NSWindow *)editorWindow {
    ASCTitleBarController *titlebarController = [self titleBarController];
    if (!titlebarController) {
        return;
    }
    NSWindow *mainWindow = titlebarController.view.window;
    if (mainWindow && mainWindow.isVisible && !mainWindow.isMiniaturized) {
        self.dropEditorWindow = editorWindow;
        
        NSPoint pos = [NSEvent mouseLocation];
        if ([titlebarController canPinTabAtPoint:pos]) {
            if (!dropTimer) {
                dropTimer = [NSTimer scheduledTimerWithTimeInterval:0.15
                                                             target:self
                                                           selector:@selector(handleDropTimer)
                                                           userInfo:nil
                                                            repeats:YES];
            }
            lastCursorPos = pos;
        } else {
            if (dropTimer) {
                [self stopDropTimer];
            }
        }
    }
}

- (void)onEditorWindowMoving:(NSNotification *)notification {
    NSWindow *editorWindow = notification.object;
    [self validateDrop:editorWindow];
}

#pragma mark -
#pragma mark Tab Detachment Support

- (void)dragDetachedTab:(NSView *)cefView atScreenPoint:(NSPoint)screenPoint withEvent:(NSEvent *)event {
    if (!self.mainWindowController) {
        return;
    }
    NSCefView *webView = (NSCefView *)cefView;
    
    NSDictionary *widgetInfo = @{@"widgetType": @"window", @"captionHeight": TOOLBTN_HEIGHT};
    [webView setParentWidgetInfoWithJson:[widgetInfo jsonString]];
    
    ASCTitleWindowController *mainWindowController = (ASCTitleWindowController *)self.mainWindowController;
    NSSize size = [mainWindowController normalFrame].size;
    NSRect windowFrame = NSMakeRect(screenPoint.x - 200, screenPoint.y - size.height + 11, size.width, size.height);
        
    ASCEditorWindowController *windowController = [ASCEditorWindowController initWithFrame:windowFrame];
    [self.editorWindowControllers addObject:windowController];
    
    ASCEditorWindow *editorWindow = (ASCEditorWindow *)windowController.window;
    [editorWindow setTitleVisibility:NSWindowTitleHidden];
    [editorWindow setTitle:[webView.data title:YES]];
    
    NSViewController *contentViewController = windowController.contentViewController;
    if (contentViewController && contentViewController.view) {
        editorWindow.webView = webView;
        [contentViewController.view addSubview:webView];
        [webView autoPinEdgesToSuperviewEdges];
    }
    
    [editorWindow makeKeyAndOrderFront:nil];
    [editorWindow.contentView layoutSubtreeIfNeeded];
    [editorWindow displayIfNeeded];
    
    NSDictionary *windowFeatures = @{@"skiptoparea": TOOLBTN_HEIGHT, @"singlewindow": @YES};
    [webView sendCommand:@"window:features" withParam:[windowFeatures jsonString]];
    [webView focus];

    // Let the event loop process before starting drag to prevent window jerking
    [editorWindow performSelector:@selector(performWindowDragWithEvent:)
                       withObject:event
                       afterDelay:0.016];
}

@end

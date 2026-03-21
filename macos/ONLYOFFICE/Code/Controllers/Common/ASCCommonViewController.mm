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
//  ASCCommonViewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCCommonViewController.h"
#import "applicationmanager.h"
#import "mac_application.h"
#import "ASCTabView.h"
#import "ASCTitleWindowController.h"
#import "ASCHelper.h"
#import "ASCConstants.h"
#import "ASCUserInfoViewController.h"
#import "NSView+Extensions.h"
#import "NSAlert+SynchronousSheet.h"
#import "NSColor+Extensions.h"
#import "NSString+Extensions.h"
#import "NSDictionary+Extensions.h"
#import "AppDelegate.h"
#import "NSCefView.h"
#import "ASCEventsController.h"
#import "ASCDownloadController.h"
#import "ASCSharedSettings.h"
#import "ASCReplacePresentationAnimator.h"
#import "AnalyticsHelper.h"
#import "PureLayout.h"
#import "NSWindow+Extensions.h"
#import "ASCExternalController.h"
#import "ASCTouchBarController.h"
#import "ASCLinguist.h"
#import "ASCThemesController.h"
#import "ASCEditorJSVariables.h"
#import "ASCPresentationReporter.h"
#import "ASCLicenseController.h"
#import <Carbon/Carbon.h>
#import <QuartzCore/QuartzCore.h>


@interface ASCCommonViewController() <ASCTabsControlDelegate, ASCTitleBarControllerDelegate, ASCUserInfoViewControllerDelegate> {

}
@property (nonatomic) NSCefView * cefStartPageView;
@property (nonatomic) BOOL shouldLogoutPortal;
@property (nonatomic) BOOL waitingForClose;
@property (nonatomic, assign) id <ASCExternalDelegate> externalDelegate;
@property (nonatomic) ASCTouchBarController *touchBarController;
@end

@implementation ASCCommonViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.tabsWithChanges = @[].mutableCopy;
    self.tabView.delegate = self;
    
    _externalDelegate = [[ASCExternalController shared] delegate];
    
    void (^addObserverFor)(_Nullable NSNotificationName, SEL) = ^(_Nullable NSNotificationName name, SEL selector) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:selector
                                                     name:name
                                                   object:nil];
    };
    
    addObserverFor(ASCEventNameMainWindowLoaded, @selector(onWindowLoaded:));
    addObserverFor(ASCEventNameOpenAppLinks, @selector(onOpenAppLink));
    addObserverFor(CEFEventNameTabEditorNameChanged, @selector(onCEFChangedTabEditorName:));
    addObserverFor(CEFEventNameTabEditorType, @selector(onCEFChangedTabEditorType:));
    addObserverFor(CEFEventNameSave, @selector(onCEFSave:));
    addObserverFor(CEFEventNameOpenUrl, @selector(onCEFOpenUrl:));
    addObserverFor(CEFEventNameFullscreen, @selector(onCEFFullscreen:));
    addObserverFor(CEFEventNameDownload, @selector(onCEFDownload:));
    addObserverFor(CEFEventNamePortalLogin, @selector(onCEFPortalLogin:));
    addObserverFor(CEFEventNamePortalLogout, @selector(onCEFPortalLogout:));
    addObserverFor(CEFEventNamePortalCreate, @selector(onCEFPortalCreate:));
    addObserverFor(CEFEventNamePortalNew, @selector(onCEFPortalNew:));
    addObserverFor(CEFEventNamePortalSSO, @selector(onCEFPortalSSO:));
    addObserverFor(CEFEventNameFilesCheck, @selector(onCEFFilesCheck:));
    addObserverFor(CEFEventNameStartPageReady, @selector(onCEFStartPageReady:));
    addObserverFor(CEFEventNameEditorAppReady, @selector(onCEFEditorAppReady:));
    addObserverFor(CEFEventNameEditorAppActionRequest, @selector(onCEFEditorAppActionRequest:));
    addObserverFor(CEFEventNameEditorOpenFolder, @selector(onCEFEditorOpenFolder:));
    addObserverFor(CEFEventNameDocumentFragmentBuild, @selector(onCEFDocumentFragmentBuild:));
    addObserverFor(CEFEventNameDocumentFragmented, @selector(onCEFDocumentFragmented:));
    
    if (_externalDelegate && [_externalDelegate respondsToSelector:@selector(onCommonViewDidLoad:)]) {
        [_externalDelegate onCommonViewDidLoad:self];
    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    
    // Update the view, if already loaded.
}

- (void)onWindowLoaded:(NSNotification *)notification {
    if (notification && notification.object) {
        ASCTitleWindowController *windowController = (ASCTitleWindowController *)notification.object;
        windowController.titlebarController.delegate = self;
        
        self.tabsControl = windowController.titlebarController.tabsControl;
        
        
        [self setupTabControl];
        [self createStartPage];
        [self loadStartPage];
        [self setupTouchBar];
        
        // External handle
        if (_externalDelegate && [_externalDelegate respondsToSelector:@selector(onMainWindowLoaded:)]) {
            [_externalDelegate onMainWindowLoaded:self];
        }
    }
}

- (void)setupTabControl {
    self.tabsControl.minTabWidth = 48;
    self.tabsControl.maxTabWidth = 135;
    
    [self.tabsControl.multicastDelegate addDelegate:self];
}

- (void)setupTouchBar {
    __weak typeof(self) weakSelf = self;
    _touchBarController = [[ASCTouchBarController alloc] init:self];
    _touchBarController.onItemTap = ^(id  _Nonnull sender, NSString * _Nonnull senderId) {
        [weakSelf onTouchBarItemTap:sender senderId:senderId];
    };
    [self.tabsControl.multicastDelegate addDelegate:_touchBarController];
    [_touchBarController invalidateTouchBar];
}

- (void)createStartPage {
    NSInteger rootTabIndex = [self.tabView indexOfTabViewItemWithIdentifier:rootTabId];
    
    if (rootTabIndex != NSNotFound) {
        NSTabViewItem * tab = [self.tabView tabViewItemAtIndex:rootTabIndex];
        
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        self.cefStartPageView = [[NSCefView alloc] initWithFrame:tab.view.frame];
        [self.cefStartPageView create:appManager withType:cvwtSimple];
        NSColor * backColor = [ASCThemesController currentThemeColor:windowBackgroundColor];
        [self.cefStartPageView setBackgroundColor:backColor];
        [tab.view addSubview:self.cefStartPageView];
        [self.cefStartPageView autoPinEdgesToSuperviewEdges];
    }
}

- (void)loadStartPage {
    if (self.cefStartPageView ) {
        id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
        
        NSUserDefaults *preferences     = [NSUserDefaults standardUserDefaults];
        NSURLComponents *loginPage      = [NSURLComponents componentsWithString:[[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"login"]];
        
        NSURLQueryItem *countryCode     = [NSURLQueryItem queryItemWithName:@"lang" value: [ASCLinguist appLanguageCode]];
        NSURLQueryItem *portalAddress   = [NSURLQueryItem queryItemWithName:@"portal" value:[preferences objectForKey:ASCUserSettingsNamePortalUrl]];
        
        if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
            countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[externalDelegate onAppPreferredLanguage]];
        }
        
        loginPage.queryItems            = @[countryCode, portalAddress];
        loginPage.scheme                = NSURLFileScheme;
        
        [self.cefStartPageView loadWithUrl:[loginPage string]];
    }
}

- (void)openLocalPage:(NSString *)path title:(NSString *)title {
    [self openLocalPage:path query:nil title:title];
}

- (void)openLocalPage:(NSString *)path query:(NSString *)query title:(NSString *)title {
    if ( !path ) return;
    
    NSURLComponents *urlPage = [NSURLComponents componentsWithString:path];
    urlPage.scheme = NSURLFileScheme;
    urlPage.query = query;

//    ASCTabView * existTab = [self tabWithParam:@"url" value:[urlPage string]];
//
//    if (existTab) {
//        [self.tabsControl selectTab:existTab];
//    } else {
//        ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
//        tab.title       = title;
//        tab.type        = ASCTabViewTypePortal;
//        tab.params      = [@{@"url" : [urlPage string]} mutableCopy];
//
//        [self.tabsControl addTab:tab selected:YES];
//    }
        
    NSWindow *mainWindow = [[NSApp windows] objectAtIndex:0];
    if (mainWindow) {
        ASCCommonViewController * controller = (ASCCommonViewController *)mainWindow.contentViewController;
        NSWindowController * windowController = [controller.storyboard instantiateControllerWithIdentifier:@"ASCLicenseWindowControllerId"];
        ASCLicenseController *licView = (ASCLicenseController *)windowController.contentViewController;
        [licView setUrl:urlPage.URL];
        NSWindow *licWindow = windowController.window;
        
        NSRect parentFrame = mainWindow.frame;
        NSRect childFrame = licWindow.frame;
        [licWindow setFrameOrigin:NSMakePoint(NSMidX(parentFrame) - childFrame.size.width/2,
                                              NSMidY(parentFrame) - childFrame.size.height/2)];
        [licWindow makeKeyAndOrderFront:nil]; // Show the window first to apply the coordinates
        
        [NSApp runModalForWindow:licWindow];
    }
}

- (void)displayPortalTabBy:(NSString *)path {
    if (NSURL * url = [NSURL URLWithString:path]) {
        NSString * urlHost = [url host];
        BOOL isFoundPortal = false;
        
        // Search opened tab of a portal
        for (ASCTabView * tab in self.tabsControl.tabs) {
            if (tab.type == ASCTabViewTypePortal) {
                if (NSString * portalUrlString = tab.params[@"url"]) {
                    if (NSURL * portalURL = [NSURL URLWithString:portalUrlString]) {
                        NSString * portalHost = [portalURL host];
                        
                        if ([portalHost isEqualToString:urlHost]) {
                            [self.tabsControl selectTab:tab];
                            
                            if (NSCefView * cefView = [self cefViewWithTab:tab]) {
                                id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
                                NSURLComponents *urlPage      = [NSURLComponents componentsWithString:path];
                                NSURLQueryItem *countryCode   = [NSURLQueryItem queryItemWithName:@"lang" value:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];
                                NSURLQueryItem *portalAddress = [NSURLQueryItem queryItemWithName:@"desktop" value:@"true"];
                                
                                if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
                                    countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[externalDelegate onAppPreferredLanguage]];
                                }
                                
                                NSMutableArray * qitems = urlPage.queryItems ? [NSMutableArray arrayWithArray:urlPage.queryItems] : [[NSMutableArray alloc] init];
                                [qitems addObjectsFromArray:@[countryCode, portalAddress]];
                                urlPage.queryItems = qitems;
                                
                                [cefView loadWithUrl:[urlPage string]];
                            }
                            
                            isFoundPortal = true;
                            break;
                        }
                    }
                }
            }
        }
        
        // Force open tab of a portal if not exist
        if (!isFoundPortal) {
            id <ASCExternalDelegate> externalDelegate = [[ASCExternalController shared] delegate];
            NSURLComponents *urlPage      = [NSURLComponents componentsWithString:path];
            NSURLQueryItem *countryCode   = [NSURLQueryItem queryItemWithName:@"lang" value:[[[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode] lowercaseString]];
            NSURLQueryItem *portalAddress = [NSURLQueryItem queryItemWithName:@"desktop" value:@"true"];
            
            if (externalDelegate && [externalDelegate respondsToSelector:@selector(onAppPreferredLanguage)]) {
                countryCode = [NSURLQueryItem queryItemWithName:@"lang" value:[externalDelegate onAppPreferredLanguage]];
            }
            
            NSMutableArray * qitems = urlPage.queryItems ? [NSMutableArray arrayWithArray:urlPage.queryItems] : [[NSMutableArray alloc] init];
            [qitems addObjectsFromArray:@[countryCode, portalAddress]];
            urlPage.queryItems = qitems;
            
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                @"action"  : @(ASCTabActionOpenPortal),
                @"url"     : [urlPage string],
                @"active"  : @(YES)
            }];
        }
    }
}

- (void)openAcknowledgments {
    NSString *language = [[NSLocale preferredLanguages] firstObject];
    
    if (language) {
        if ([language length] > 1) {
            language = [language substringToIndex:2];
        }
        
        NSString *query = [NSString stringWithFormat:@"lang=%@", language.lowercaseString];
        [self openLocalPage:[[NSBundle mainBundle] pathForResource:@"acknowledgments" ofType:@"html" inDirectory:@"login"]
                      query:query
                      title:NSLocalizedString(@"Acknowledgments", nil)];
    } else {
        [self openLocalPage:[[NSBundle mainBundle] pathForResource:@"acknowledgments" ofType:@"html" inDirectory:@"login"]
                      title:NSLocalizedString(@"Acknowledgments", nil)];
    }
}

- (void)openEULA {
    NSString * eulaUrl = [[NSBundle mainBundle] pathForResource:@"EULA" ofType:@"html" inDirectory:@"license"];
    if ( !eulaUrl )
        eulaUrl = [[NSBundle mainBundle] pathForResource:@"LICENSE" ofType:@"html" inDirectory:@"license"];

    if ( eulaUrl ) {
        [self openLocalPage:eulaUrl title:NSLocalizedString(@"License Agreement", nil)];
    }
}

- (void)openPreferences {
    [self.tabView selectTabViewItemWithIdentifier:rootTabId];
    [self.tabsControl selectTab:nil];
    
    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
    pCommand->put_Command(L"panel:select");
    pCommand->put_Param(L"settings");
    
    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;
    
    [self.cefStartPageView apply:pEvent];
}

- (void)onOpenAppLink {
    if (NSArray<NSURL *> * links = [[ASCSharedSettings sharedInstance] settingByKey:kSettingsOpenAppLinks]) {
        [links enumerateObjectsUsingBlock:^(NSURL * _Nonnull link, NSUInteger idx, BOOL * _Nonnull stop) {
            ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
            tab.type        = ASCTabViewTypeOpening;
            tab.params      = [@{
                @"action": @(ASCTabActionOpenUrl),
                @"title": [NSString stringWithFormat:@"%@...", NSLocalizedString(@"Opening", nil)],
                @"url": [link.absoluteString stringByRemovingPercentEncoding]
            } mutableCopy];
            
            [self.tabsControl addTab:tab selected:YES];
        }];
        
        [[ASCSharedSettings sharedInstance] setSetting:nil forKey:kSettingsOpenAppLinks];
    }
}

#pragma mark -
#pragma mark TouchBar

- (NSTouchBar *)makeTouchBar {
    if (@available(macOS 10.12.2, *)) {
        if (_touchBarController) {
            return [_touchBarController makeTouchBar];
        }
    }
    return nil;
}

- (void)onTouchBarItemTap:(id)sender senderId:(NSString *)senderId {
    //    NSLog(@"onTouchBarItemTap: %@", senderId);
    
    if (senderId) {
        if ([senderId isEqualToString:kStartPageButtonIdentifier]) {
            [self onOnlyofficeButton:nil];
        } else if ([senderId isEqualToString:[NSString stringWithFormat:kCreationButtonIdentifier, @"document"]]) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                @"action"  : @(ASCTabActionCreateLocalFile),
                @"type"    : @(int(AscEditorType::etDocument)),
                @"active"  : @(YES)
            }];
        } else if ([senderId isEqualToString:[NSString stringWithFormat:kCreationButtonIdentifier, @"spreadsheet"]]) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                @"action"  : @(ASCTabActionCreateLocalFile),
                @"type"    : @(int(AscEditorType::etSpreadsheet)),
                @"active"  : @(YES)
            }];
        } else if ([senderId isEqualToString:[NSString stringWithFormat:kCreationButtonIdentifier, @"presentation"]]) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                @"action"  : @(ASCTabActionCreateLocalFile),
                @"type"    : @(int(AscEditorType::etPresentation)),
                @"active"  : @(YES)
            }];
        } else if ([senderId isEqualToString:[NSString stringWithFormat:kCreationButtonIdentifier, @"pdfform"]]) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                @"action"  : @(ASCTabActionCreateLocalFile),
                @"type"    : @(int(AscEditorType::etDocumentMasterForm)),
                @"active"  : @(YES)
            }];
        } else {
            ASCTabView * tab = [self.tabsControl tabWithUUID:senderId];
            if (tab) {
                [self.tabsControl selectTab:tab];
            }
        }
    }
}

#pragma mark -
#pragma mark Public

- (BOOL)shouldCloseWindow {
    NSInteger unsaved = 0;
    //    BOOL preventTerminate = NO;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO),
                                                                 @"terminate"  : @(YES)
                                                               }];
    
    NSMutableArray * locked_uuids = [NSMutableArray array];
    for (ASCTabView * tab in self.tabsControl.tabs) {
        if (NSCefView * cefView = [self cefViewWithTab:tab]) {
            if ([cefView.data hasChanges]) {
                unsaved++;
            }

            // Blockchain check
            if ([cefView checkCloudCryptoNeedBuild]) {
                self.waitingForClose = YES;
                return NO;
            } else {
                if ([cefView isSaveLocked]) {
                    unsaved++;
                    [locked_uuids addObject:tab.uuid];
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
            // "Review Changes..." clicked
            
            self.waitingForClose = YES;
            [self.tabsWithChanges removeAllObjects];
            NSMutableArray<ASCTabView *> *tabsWithChanges = [NSMutableArray array];
            NSArray * tabs = [NSArray arrayWithArray:self.tabsControl.tabs];
            for (ASCTabView * tab in tabs) {
                NSCefView * cefView = [self cefViewWithTab:tab];
                if ((cefView && [cefView.data hasChanges]) || [locked_uuids containsObject:tab.uuid]) {
                    [tabsWithChanges addObject:tab];
                } else {
                    [self.tabsControl removeTab:tab selected:NO animated:NO];
                }
            }
            
            self.tabsWithChanges = [NSMutableArray arrayWithArray:tabsWithChanges];
            [self safeCloseTabsWithChanges];
        } else if (result == NSAlertSecondButtonReturn) {
            // "Cancel" clicked
            return NO;
        } else {
            // "Delete and Quit" clicked
            
            self.waitingForClose = YES;
            NSArray * tabs = [NSArray arrayWithArray:self.tabsControl.tabs];
            
            for (ASCTabView * tab in tabs) {
                [self.tabsControl removeTab:tab selected:NO animated:NO];
            }
            
            [self.tabView selectTabViewItemWithIdentifier:rootTabId];
        }
        
        return NO;

    } else {
        [self.tabsWithChanges removeAllObjects];
        NSArray * tabs = [NSArray arrayWithArray:self.tabsControl.tabs];
        for (ASCTabView * tab in tabs) {
            [self.tabsControl removeTab:tab selected:NO animated:NO];
        }
        [self.tabView selectTabViewItemWithIdentifier:rootTabId];
    }
    
    return YES;
}

- (BOOL)shouldCloseWindowIfNoTabs {
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO)}];
    
    if (self.tabsControl.tabs.count > 0) {
        ASCTabView * tab = [self.tabsControl selectedTab];
        
        if (tab) {
            if ([self tabs:self.tabsControl willRemovedTab:tab]) {
                [self.tabsControl removeTab:tab animated:NO];
            }
            return NO;
        }
    }
    return YES;
}

- (NSCefView *)cefViewWithTab:(ASCTabView *)tab {
    NSTabViewItem * item = nil;
    NSCefView * cefView = nil;
    
    if (tab) {
        item = [self.tabView tabViewItemAtIndex:[self.tabView indexOfTabViewItemWithIdentifier:tab.uuid]];
    } else {
        item = [self.tabView tabViewItemAtIndex:[self.tabView indexOfTabViewItemWithIdentifier:rootTabId]];
    }
    
    if (item) {
        for (NSView * view in item.view.subviews) {
            if ([view isKindOfClass:[NSCefView class]]) {
                cefView = (NSCefView *)view;
                break;
            }
        }
    }
    
    return cefView;
}

- (ASCTabView *)tabViewWithId:(int)viewId {
    return [self.tabsControl tabWithUUID:[NSString stringWithFormat:@"%d", viewId]];
}

#pragma mark -
#pragma mark Internal

- (ASCTabView *)tabWithParam:(NSString *)param value:(NSString *)value {
    if (param && value && [value isKindOfClass:[NSString class]] && value.length > 0) {
        for (ASCTabView * tab in self.tabsControl.tabs) {
            if (tab.params[param] && [tab.params[param] isKindOfClass:[NSString class]]) {
                NSString * localTabValue = [NSString stringWithString:tab.params[param]];
                NSString * localValue = [NSString stringWithString:value];
                
                if (NSString *provider = tab.params[@"provider"]) {
                    if ([@[@"asc", @"onlyoffice"] containsObject:provider]) {
                        if ([localTabValue isEqualToString:localValue]) {
                            return tab;
                        }
                        localValue = [localValue stringByReplacingOccurrencesOfString:@"/products/files/" withString:@""];
                    }
                }
                
                localTabValue = [localTabValue removeUrlQuery:@[@"lang", @"desktop"]];
                localValue = [localValue removeUrlQuery:@[@"lang", @"desktop"]];
                
                if (localTabValue && localTabValue.length > 0 && [localTabValue isEqualToString:localValue]) {
                    return tab;
                }
            }
        }
    }
    return nil;
}

- (NSDictionary *)checkFiles:(NSDictionary *)fileList {
    NSMutableDictionary * checkedList = @{}.mutableCopy;
    
    for (NSString * key in fileList) {
        if (NSString * pathString = [fileList objectForKey:key]) {
            NSRange typeRange = [pathString rangeOfString:@"^https?://"
                                                  options:NSRegularExpressionSearch];
            
            if (typeRange.location != NSNotFound) {
                checkedList[key] = @"true";
            } else {
                checkedList[key] = [[NSFileManager defaultManager] fileExistsAtPath:pathString] ? @"true" : @"false";
            }
        }
    }
    
    return checkedList;
}

- (void)requestSaveChangesForTab:(ASCTabView *)tab {
    if (tab) {
        NSCefView * cefView = [self cefViewWithTab:tab];
        if (cefView && ([cefView.data hasChanges] || [cefView isSaveLocked])) {
            [self.view.window makeKeyAndOrderFront:nil];

            NSAlert *alert = [[NSAlert alloc] init];
            [alert addButtonWithTitle:NSLocalizedString(@"Save", nil)];
            [alert addButtonWithTitle:NSLocalizedString(@"Don't Save", nil)];
            [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
            [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"Do you want to save the changes made to the document \"%@\"?", nil), [cefView.data title:YES]]];
            [alert setInformativeText:NSLocalizedString(@"Your changes will be lost if you don’t save them.", nil)];
            [alert setAlertStyle:NSAlertStyleWarning];
            
            [self.tabsControl selectTab:tab];
            
            NSInteger returnCode = [alert runModalSheet];
            
            if(returnCode == NSAlertFirstButtonReturn) {
                tab.params[@"shouldClose"] = @(YES);
                
                if (cefView) {
                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE);
                    [cefView apply:pEvent];
                }
            } else if (returnCode == NSAlertSecondButtonReturn) {
                [self.tabsControl removeTab:tab animated:NO];
            } else if (returnCode == NSAlertThirdButtonReturn) {
                AppDelegate *app = (AppDelegate *)[NSApp delegate];
                app.waitingForTerminateApp = NO;
                self.waitingForClose = NO;
                self.shouldLogoutPortal = NO;
                [self.tabsWithChanges removeAllObjects];
            }
        }
    }
}

- (void)safeCloseTabsWithChanges {
    if ([[self tabsWithChanges] count] > 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self tabs:self.tabsControl willRemovedTab:[self.tabsWithChanges firstObject]];
        });
    }
}

#pragma mark -
#pragma mark CEF events handlers

- (void)onCEFChangedTabEditorName:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
        if (tab) {
        }
    }
}

- (void)onCEFChangedTabEditorType:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        //        NSInteger type          = [params[@"type"] integerValue];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
        if (tab) {
        }
    }
}

- (void)onCEFSave:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        
        if ( ![params[@"cancel"] boolValue] ) {
            NSString * viewId = params[@"viewId"];
            ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
            
            if (tab && tab.params[@"shouldClose"] && [tab.params[@"shouldClose"] boolValue]) {
                [self.tabsControl removeTab:tab animated:NO];
            }
        }
    }
}

- (void)onCEFOpenUrl:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * linkUrl = params[@"url"];
        
        if (linkUrl && linkUrl.length > 0) {
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:linkUrl]];
        }
    }
}

- (void)onCEFFullscreen:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        
        BOOL isFullscreen = [params[@"fullscreen"] boolValue];
        ASCTabView * tab= nil;
        if ( [params objectForKey:@"viewId"] ) {
            tab = [self tabViewWithId:[params[@"viewId"] intValue]];
        } else if ( [params objectForKey:@"terminate"] and [params[@"terminate"] boolValue] ) {
            [ASCEventsController resetFullscreenState];
            if (self.tabsControl.tabs.count > 0)
                tab = [self.tabsControl selectedTab];
        }
        
        if ( tab ) {
            NSTabViewItem * item = [self.tabView tabViewItemAtIndex:[self.tabView indexOfTabViewItemWithIdentifier:tab.uuid]];
            
            NSWindow * win_main = [NSWindow titleWindowOrMain];
            if (isFullscreen) {
                NSScreen * ppeScreen = [win_main screen];
                NSArray<NSScreen *> * screens = [NSScreen screens];
                if ( [[ASCPresentationReporter sharedInstance] isVisible] && [screens count] > 1 )
                    for ( NSScreen * screen in screens ) {
                        if ( screen != [NSScreen mainScreen] ) {
                            ppeScreen = screen;
                            break;
                        }
                    }
                
                NSApplicationPresentationOptions options = NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar;
                [item.view enterFullScreenMode:ppeScreen withOptions:@{
                    NSFullScreenModeApplicationPresentationOptions: @(options),
                    NSFullScreenModeAllScreens: @(NO)
                }];
                if ( tab ) {
                    [[self cefViewWithTab:tab] focus];
                }
                
                [win_main setIsVisible:false];
            } else if ([item.view isInFullScreenMode]) {
                [win_main setIsVisible:true];
                [item.view exitFullScreenModeWithOptions:nil];
                
                if (tab) {
                    ASCTabView * currTab = [self.tabsControl selectedTab];
                    if ( currTab.uuid != tab.uuid ) {
                        [self.tabsControl selectTab:tab];
                    }
                    
                    NSCefView * cefView = [self cefViewWithTab:tab];
                    
                    if (cefView) {
                        NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                        pCommand->put_Command(L"editor:stopDemonstration");
                        
                        NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                        pEvent->m_pData = pCommand;
                        
                        [cefView apply:pEvent];
                    }
                }
            }
        }
    }
}

- (void)onCEFDownload:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        NSValue * eventData = params[@"data"];
        
        if (nil == appManager || nil == eventData)
            return;
        
        NSEditorApi::CAscDownloadFileInfo * pDownloadFileInfo = (NSEditorApi::CAscDownloadFileInfo *)[eventData pointerValue];
        
        NSString * idx = [NSString stringWithFormat:@"%d", pDownloadFileInfo->get_Id()];
        
        if (pDownloadFileInfo->get_IsComplete()) {
            [[ASCDownloadController sharedInstance] removeDownload:idx];
        } else if (pDownloadFileInfo->get_IsCanceled()) {
            [[ASCDownloadController sharedInstance] removeDownload:idx];
        } else {
            id download = [[ASCDownloadController sharedInstance] downloadWithId:idx];
            
            if (nil == download) {
                NSString * path = [NSString stringWithstdwstring:pDownloadFileInfo->get_FilePath()];
                NSString * fileName = NSLocalizedString(@"Unconfirmed", nil);
                
                if (path && [path length] > 0) {
                    fileName = [path lastPathComponent];
                }
                
                [[ASCDownloadController sharedInstance] addDownload:idx fileName:fileName];
            }
            
            [[ASCDownloadController sharedInstance] updateDownload:idx data:eventData];
        }
    }
}

- (void)onCEFPortalLogin:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        if (NSString *viewId = json[@"viewId"]) {
            if (ASCTabView * tab = [self.tabsControl tabWithUUID:viewId]) {
                if (NSCefView * cefView = [self cefViewWithTab:tab]) {
                    if (NSString *infoString = json[@"info"]) {
                        if (NSMutableDictionary *info = [[infoString dictionary] mutableCopy]) {
                            if (NSString *originalUrl = [cefView originalUrl]) {
                                originalUrl = [[originalUrl stringByReplacingOccurrencesOfString:@"://" withString:@":////"]
                                               stringByReplacingOccurrencesOfString:@"//" withString:@"/"];
                                
                                // Hotfix virtual path
                                if (NSString *provider = tab.params[@"provider"]) {
                                    if ([@[@"asc", @"onlyoffice"] containsObject:provider]) {
                                        originalUrl = [originalUrl stringByReplacingOccurrencesOfString:@"/products/files/" withString:@""];
                                    }
                                }
                                originalUrl = [originalUrl virtualUrl];
                                
                                if (NSString * jsonString = [info jsonString]) {
                                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                                    pCommand->put_Command(L"portal:login");
                                    pCommand->put_Param([jsonString stdwstring]);
                                    
                                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                                    pEvent->m_pData = pCommand;
                                    
                                    [self.cefStartPageView apply:pEvent];
                                    
                                    // Hotfix for SSO
                                    tab.params[@"url"] = originalUrl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

- (void)onCEFPortalLogout:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        NSString * portal = json[@"domain"];
        NSMutableArray * portals = [NSMutableArray arrayWithObject:portal];
        
        if (portal) {
            BOOL isReload = [@"reload" isEqualToString:json[@"onsuccess"]];
            self.shouldLogoutPortal = YES;
            
            if ( json[@"extra"] != nil ) {
                NSArray * urls = [json valueForKey:@"extra"];
                
                for ( NSString * u in urls ) {
                    [portals addObject:u];
                }
            }
            
            bool (^_is_array_contains_url)(NSArray *, NSString *) = ^(NSArray * arr, NSString * s) {
                if ( s && s.length )
                    for ( NSString * u in arr ) {
                        if ( [s rangeOfString:u].location != NSNotFound )
                            return true;
                    }
                
                return false;
            };
            
            NSMutableArray * portalTabs = [NSMutableArray array];
            NSMutableArray * saveLockedTabs = [NSMutableArray array];
            NSInteger unsaved = 0;
            
            for (ASCTabView * tab in self.tabsControl.tabs) {
                NSString * tabVirtualUrl = [tab.params[@"url"] virtualUrl];
                
                if ( !tabVirtualUrl ) {
                    NSString * pathString = tab.params[@"path"];
                    NSRange typeRange = [pathString rangeOfString:@"^https?://"
                                                          options:NSRegularExpressionSearch];
                    
                    if ( typeRange.location != NSNotFound )
                        tabVirtualUrl = pathString;
                }
                
                if ( _is_array_contains_url(portals, tabVirtualUrl) ) {
                    NSCefView * cefView = [self cefViewWithTab:tab];
                    if ( isReload ) {
                        if ( cefView ) {
                            [cefView reload];
                        }
                    } else {
                        [portalTabs addObject:tab];
                        
                        if (cefView && ([cefView.data hasChanges] || [cefView isSaveLocked])) {
                            unsaved++;
                            [saveLockedTabs addObject:tab.uuid];
                        }
                    }
                }
            }
            
            if ( !isReload ) {
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
                        // "Review Changes..." clicked
                        
                        [self.tabsWithChanges removeAllObjects];
                        NSMutableArray<ASCTabView *> *tabsWithChanges = [NSMutableArray array];
                        for (ASCTabView * tab in portalTabs) {
                            NSCefView * cefView = [self cefViewWithTab:tab];
                            if ((cefView && [cefView.data hasChanges]) || [saveLockedTabs containsObject:tab.uuid]) {
                                [tabsWithChanges addObject:tab];
                            } else {
                                [self.tabsControl removeTab:tab selected:NO animated:NO];
                            }
                        }
                        
                        self.tabsWithChanges = [NSMutableArray arrayWithArray:tabsWithChanges];
                        [self safeCloseTabsWithChanges];
                    } else if (result == NSAlertSecondButtonReturn) {
                        return;
                    } else {
                        // "Delete and Quit" clicked
                        
                        for (ASCTabView * tab in portalTabs) {
                            [self.tabsControl removeTab:tab selected:NO animated:NO];
                        }
                    }
                } else {
                    for (ASCTabView * tab in portalTabs) {
                        [self.tabsControl removeTab:tab selected:NO animated:NO];
                    }
                    
                    [self.tabView selectTabViewItemWithIdentifier:rootTabId];
                }
            }
        }
        
        if (self.shouldLogoutPortal) {
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            
            for ( NSString * u in portals ) {
                appManager->Logout([u stdwstring]);
            }
            
            NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
            pCommand->put_Command(L"portal:logout");
            pCommand->put_Param([portal stdwstring]);
            
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
            pEvent->m_pData = pCommand;
            
            [self.cefStartPageView apply:pEvent];
        }
    }
}

- (void)onCEFPortalCreate:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                        object:nil
                                                      userInfo:@{
        @"action"  : @(ASCTabActionOpenPortal),
        @"url"     : [ASCConstants appInfo:kRegistrationPortalUrl],
        @"title"   : NSLocalizedString(@"Create portal", nil),
        @"active"  : @(YES)
    }];
}


- (void)onCEFPortalNew:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        NSString * domainName = json[@"domain"];
        
        if (domainName && domainName.length > 0) {
            domainName = [[NSURL URLWithString:domainName] host];
        }
        
        ASCTabView * existTab = [self tabWithParam:@"url" value:[ASCConstants appInfo:kRegistrationPortalUrl]];
        
        if (existTab) {
            existTab.params[@"url"] = domainName;
            existTab.params[@"title"] = domainName;
            existTab.title = domainName;
            [self.tabsControl selectTab:existTab];
        }
    }
}

- (void)onCEFPortalSSO:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        NSString * portalUrl = json[@"portal"];
        NSString * providerUrl = json[@"provider"];
        
        if (portalUrl && providerUrl) {
            ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
            tab.title       = portalUrl;
            tab.type        = ASCTabViewTypePortal;
            tab.params      = [@{@"url" : [NSString stringWithFormat:@"sso:%@", providerUrl],
                                 @"provider": @"asc"
                               } mutableCopy];
            
            [self.tabsControl addTab:tab selected:YES];
        }
    }
}

- (void)onCEFFilesCheck:(NSNotification *)notification {
    AppDelegate *app = (AppDelegate *)[NSApp delegate];
    if (app.waitingForTerminateApp || self.waitingForClose) {
        return;
    }
    
    if (notification && notification.userInfo) {
        id paths = notification.userInfo;
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            NSDictionary * checkedList = [self checkFiles:paths];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                if (NSString * jsonString = [checkedList jsonString]) {
                    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                    pCommand->put_Command(L"files:checked");
                    pCommand->put_Param([jsonString stdwstring]);
                    
                    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
                    pEvent->m_pData = pCommand;
                    
                    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                    appManager->SetEventToAllMainWindows(pEvent);
                }
            });
        });
    }
}

- (void)onCEFStartPageReady:(NSNotification *)notification {
    
    NSString * uiTheme = [[NSUserDefaults standardUserDefaults] valueForKey:ASCUserUITheme] ?:
    [ASCThemesController defaultThemeId:[ASCThemesController isCurrentThemeDark]];
    
    NSMutableDictionary * json_langs = @{
        @"uitheme": uiTheme,
        @"rtl": @([ASCLinguist isUILayoutDirectionRtl])
    }.mutableCopy;
    
    NSDictionary * langs = [ASCLinguist availableLanguages];
    if ( langs ) {
        [json_langs setObject:@{
            @"current": [ASCLinguist appLanguageCode],
            @"langs": langs,
            @"restart": @true
        } forKey:@"locale"];
    }
    
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    bool usegpu = !(appManager->GetUserSettings()->Get(L"disable-gpu") == L"1"),
    useai = !(appManager->GetUserSettings()->Get(L"disable-ai") == L"1"),
    editorinwindow = appManager->GetUserSettings()->Get(L"editor-window-mode") == L"1",
    detectkeyboard = !(appManager->GetUserSettings()->Get(L"spell-check-input-mode") == L"0");
    [json_langs setValue:@(usegpu) forKey:@"usegpu"];
    [json_langs setValue:@(useai) forKey:@"useai"];
    [json_langs setValue:detectkeyboard?@"auto":@"off" forKey:@"spellcheckdetect"];
    [json_langs setValue:@(editorinwindow) forKey:@"editorwindowmode"];

    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
    pCommand->put_Command(L"settings:init");
    pCommand->put_Param([[json_langs jsonString] stdwstring]);
    
    NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;
    pEvent->AddRef();
    
    [self.cefStartPageView apply:pEvent];
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:ASCUserLockPageConnections]) {
        pCommand->put_Command(L"panel:hide");
        pCommand->put_Param(L"connect");
        
        pEvent->AddRef();
        [self.cefStartPageView apply:pEvent];
    }
    
    pCommand->put_Command(L"app:ready");
    pCommand->put_Param(L"");
    
    [self.cefStartPageView apply:pEvent];
    
    [self onOpenAppLink];
}

- (void)onCEFEditorAppReady:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        if (NSString * viewId = json[@"viewId"]) {
        }
    }
}

- (void)onCEFEditorAppActionRequest:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        NSString * action = json[@"action"];
        NSString * path = json[@"url"];
        
        if (viewId && action) {
            if (ASCTabView * tab = [self.tabsControl tabWithUUID:viewId]) {
                if ([action isEqualToString:@"close"]) {
                    [self.tabsControl removeTab:tab selected:NO animated:NO];
                }
            }
            
            // Open start page or portal
            if (path) {
                if ([path isEqualToString:@"onlyoffice.com"]) { // onlyoffice.com equal "offline"
                    [self.tabView selectTabViewItemWithIdentifier:rootTabId];
                    [self.tabsControl selectTab:nil];
                } else {
                    [self displayPortalTabBy:path];
                }
            }
        }
    }
}

- (void)onCEFEditorOpenFolder:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        NSString * path = json[@"path"];
        
        if (viewId && path) {
            if ([path isEqualToString:@"offline"]) {
                int cefViewId = [viewId intValue];
                CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
                CCefViewEditor * cefView = (CCefViewEditor *)appManager->GetViewById(cefViewId);
                
                if (cefView) {
                    NSString * urlString = [NSString stringWithstdwstring:cefView->GetLocalFilePath()];
                    
                    if (urlString && urlString.length > 0) {
                        // Offline file is exist
                        if (NSURL * url = [NSURL fileURLWithPath:urlString]) {
                            [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:@[url]];
                        }
                    } else {
                        // Offline file is new
                        
                        NSAlert *alert = [NSAlert new];
                        [alert addButtonWithTitle:@"OK"];
                        [alert setMessageText:NSLocalizedString(@"Cannot open folder of the file location.", nil)];
                        [alert setInformativeText:NSLocalizedString(@"To open the file location, it must be saved.", nil)];
                        [alert setAlertStyle:NSAlertStyleWarning];
                        
                        [alert runModalSheet];
                    }
                }
            } else {
                [self.view.window makeKeyAndOrderFront:nil];
                [self displayPortalTabBy:path];
            }
        }
    }
}

- (void)onCEFDocumentFragmentBuild:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        int error = [json[@"error"] intValue];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        if (tab) {
            if (error == 0) {
                [self.tabsControl removeTab:tab selected:YES animated:NO];
            }
        }
    }
}

- (void)onCEFDocumentFragmented:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        BOOL isFragmented = [json[@"isFragmented"] boolValue];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        if (tab) {
            if (isFragmented) {
                //                windows code skip this warning for ver 5.2
                //                NSAlert * alert = [NSAlert new];
                //                [alert addButtonWithTitle:NSLocalizedString(@"Yes", nil)];
                //                [alert addButtonWithTitle:NSLocalizedString(@"No", nil)];
                //                [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
                //                [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"The document \"%@\" must be built. Continue?", nil), [tab title]]];
                //                [alert setAlertStyle:NSAlertStyleInformational];
                //
                //                NSInteger returnCode = [alert runModalSheet];
                //
                //                if (returnCode == NSAlertFirstButtonReturn) {
                NSCefView * cefView = [self cefViewWithTab:tab];
                
                if (cefView) {
                    //                        self.shouldTerminateApp = NO;
                    
                    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD);
                    [cefView apply:pEvent];
                    return;
                }
                //                } else if (returnCode == NSAlertSecondButtonReturn) {
                //                    //
                //                } else {
                //                    return;
                //                }
            }
            
            [self.tabsControl removeTab:tab selected:YES animated:NO];
        }
    }
}

#pragma mark -
#pragma mark ASCTabsControl Delegate

- (void)tabs:(ASCTabsControl *)control didSelectTab:(ASCTabView *)tab {
    [[ASCSharedSettings sharedInstance] setSetting:tab forKey:kSettingsCurrentTab];
    
    if (tab) {
        [self.tabView selectTabViewItemWithIdentifier:tab.uuid];
    } else {
        NSTabViewItem * item = [self.tabView selectedTabViewItem];
        if ( ![[item identifier]  isEqual:rootTabId] ) {
            [self.tabView selectTabViewItemWithIdentifier:rootTabId];
        }
    }
}

//- (void)tabs:(ASCTabsControl *)control didUpdateTab:(ASCTabView *)tab {
//    NSLog(@"didUpdateTab");
//}

- (void)tabs:(ASCTabsControl *)control didAddTab:(ASCTabView *)tab {
    if (tab.params) {
        BOOL isReattaching = [tab.params[@"reattaching"] boolValue];
        NSCefView *existingCefView = (NSCefView *)tab.webView;

        if (isReattaching && existingCefView) {
            tab.uuid = [NSString stringWithFormat:@"%ld", (long)existingCefView.uuid];

            NSTabViewItem *item = [[NSTabViewItem alloc] initWithIdentifier:tab.uuid];
            item.label = tab.title;
            [self.tabView addTabViewItem:item];
            [item.view addSubview:existingCefView];
            [existingCefView autoPinEdgesToSuperviewEdges];

            [self tabView:self.tabView dimTabViewItem:item];
            return;
        }

        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        NSCefView * cefView = [[NSCefView alloc] initWithFrame:CGRectZero];
        
        ASCTabActionType action = (ASCTabActionType)[tab.params[@"action"] intValue];
        CefViewWrapperType type = (action == ASCTabActionOpenPortal) ? cvwtSimple : cvwtEditor;

        NSCefData *cefData = [[NSCefData alloc] initWith: tab.title viewType:type];
        cefView.data = cefData;

        [cefView create:appManager withType:type];
        [cefView setBackgroundColor:[ASCThemesController currentThemeColor:windowBackgroundColor]];
        
        NSDictionary *widgetInfo = @{@"widgetType": @"tab", @"captionHeight": @0};
        [cefView setParentWidgetInfoWithJson:[widgetInfo jsonString]];

        [self.view.window makeKeyAndOrderFront:nil];
        
        switch (action) {
            case ASCTabActionOpenPortal: {
                tab.type = ASCTabViewTypePortal;
                
                NSString * newTitle = [[NSURL URLWithString:tab.params[@"url"]] host];
                NSString * provider = tab.params[@"provider"];
                
                if (newTitle && newTitle.length > 0) {
                    tab.title = [[NSURL URLWithString:tab.params[@"url"]] host];
                }
                
                if (provider && [provider length] > 0) {
                    [cefView setExternalCloud:provider];
                }
            }
            case ASCTabActionOpenUrl: {
                [cefView loadWithUrl:tab.params[@"url"]];
                
                if (tab.params[@"title"] && [tab.params[@"title"] length] > 0) {
                    tab.title = tab.params[@"title"];
                }
                
                break;
            }
                
            case ASCTabActionCreateLocalFileFromTemplate:
            case ASCTabActionCreateLocalFile: {
                AscEditorType docType = AscEditorType::etDocument;
                if ( [tab.params[@"type"] isKindOfClass:[NSString class]] ) {
                    NSString * param = tab.params[@"type"];
                    if ([param isEqualToString:@"cell"]) docType = AscEditorType::etSpreadsheet;
                    else if ([param isEqualToString:@"slide"]) docType = AscEditorType::etPresentation;
                    else if ([param isEqualToString:@"form"]) docType = AscEditorType::etDocumentMasterForm;
                    //                    else /*if ([param isEqualToString:@"word"])*/ docType = AscEditorType::etDocument;
                } else docType = (AscEditorType)[tab.params[@"type"] intValue];
                
                NSString * docName = NSLocalizedString(@"Untitled", nil);
                AppDelegate *app = [NSApp delegate];
                switch (docType) {
                    case AscEditorType::etDocument:
                        docName = [NSString stringWithFormat:NSLocalizedString(@"Document %ld.docx", nil), ++app.documentNameCounter];
                        break;
                    case AscEditorType::etSpreadsheet:
                        docName = [NSString stringWithFormat:NSLocalizedString(@"Spreadsheet %ld.xlsx", nil), ++app.spreadsheetNameCounter];
                        break;
                    case AscEditorType::etPresentation:
                        docName = [NSString stringWithFormat:NSLocalizedString(@"Presentation %ld.pptx", nil), ++app.presentationNameCounter];
                        break;
                    case AscEditorType::etPdf:
                    case AscEditorType::etDocumentMasterOForm:
                    case AscEditorType::etDocumentMasterForm:
                        docName = [NSString stringWithFormat:NSLocalizedString(@"Document %ld.pdf", nil), ++app.pdfNameCounter];
                        break;
                    default: break;
                }
                
                if (action == ASCTabActionCreateLocalFile ) {
                    [cefView createFileWithName:docName type:docType];
                } else {
                    [cefView createFileWithNameFromTemplate:docName tplpath:tab.params[@"path"]];
                }
                
                break;
            }
                
            case ASCTabActionOpenLocalFile: {
                NSString * filePath = tab.params[@"path"];
                
                int fileFormatType = CCefViewEditor::GetFileFormat([filePath stdwstring]);
                [cefView openFileWithName:filePath type:fileFormatType];
                
                [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                         action:@"Open local file"
                                                                          label:nil
                                                                          value:nil];
                
                break;
            }
            case ASCTabActionOpenLocalRecoverFile: {
                NSInteger docId = [tab.params[@"fileId"] intValue];
                [cefView openRecoverFileWithId:docId];
                
                [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                         action:@"Open local recover file"
                                                                          label:nil
                                                                          value:nil];
                
                break;
            }
            case ASCTabActionOpenLocalRecentFile: {
                NSInteger docId = [tab.params[@"fileId"] intValue];
                
                if ( !(docId < 0) )
                    [cefView openRecentFileWithId:docId];
                else {
                    NSString * filePath = tab.params[@"path"];
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
        
        
        tab.uuid = [NSString stringWithFormat:@"%ld", (long)cefView.uuid];
        
        // Store the cefView reference in params for later access
        tab.webView = cefView;

        NSTabViewItem * item = [[NSTabViewItem alloc] initWithIdentifier:tab.uuid];
        item.label = tab.title;
        [self.tabView addTabViewItem:item];
        [item.view addSubview:cefView];
        [cefView autoPinEdgesToSuperviewEdges];
        
        [self tabView:self.tabView dimTabViewItem:item];
    }
}

- (BOOL)tabs:(ASCTabsControl *)control willRemovedTab:(ASCTabView *)tab {
    if (tab) {
        NSCefView * cefView = [self cefViewWithTab:tab];
        if (cefView && ([cefView checkCloudCryptoNeedBuild] || [cefView checkBuilding])) {
            AppDelegate *app = (AppDelegate *)[NSApp delegate];
            app.waitingForTerminateApp = NO;
            self.waitingForClose = NO;
            return NO;
        }
        
        if (cefView && ([cefView.data hasChanges] || [cefView isSaveLocked])) {
            [self requestSaveChangesForTab:tab];
            return NO;
        }
    }
    return YES;
}

- (void)tabs:(ASCTabsControl *)control didRemovedTab:(ASCTabView *)tab {
    BOOL isDetached = [tab.params[@"detached"] boolValue];

    if (!isDetached) {
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        appManager->DestroyCefView([tab.uuid intValue]);

        NSCefView * cefView = [self cefViewWithTab:tab];

        if (cefView) {
            [cefView internalClean];
        }
    }
    
    [self.tabView removeTabViewItem:[self.tabView tabViewItemAtIndex:[self.tabView indexOfTabViewItemWithIdentifier:tab.uuid]]];
    
    if ([self.tabsWithChanges containsObject:tab]) {
        [self.tabsWithChanges removeObject:tab];
    }
    if (self.tabsWithChanges.count > 0) {
        [self safeCloseTabsWithChanges];
    }
    
    if (self.tabsControl.tabs.count < 1) {
        AppDelegate *app = (AppDelegate *)[NSApp delegate];
        if (self.waitingForClose || app.waitingForTerminateApp)
            [self.view.window close];

        if (app.waitingForTerminateApp && app.editorWindowControllers.count > 0) {
            [app safeCloseEditorWindows];
        }
    }
}

- (void)tabs:(ASCTabsControl *)control didReorderTab:(ASCTabView *)tab from:(NSInteger)oldIndex to:(NSInteger)newIndex {
    //
}

- (void)tabs:(ASCTabsControl *)control didRequestCloseTab:(ASCTabView *)tab {
    if (tab) {
        if ([self tabs:control willRemovedTab:tab]) {
            [control removeTab:tab animated:YES];
        }
    }
}

- (void)tabs:(ASCTabsControl *)control didRequestCloseSavedTabs:(ASCTabView *)tab {
    NSArray *tabsCopy = [NSArray arrayWithArray:control.tabs];
    for (ASCTabView *tab in tabsCopy) {
        NSString *path = tab.params[@"path"];
        NSCefView *cefView = [self cefViewWithTab:tab];
        if (cefView && [cefView.data isViewType:cvwtEditor] && ![cefView.data hasChanges]
                && ![cefView isSaveLocked] && path && path.length > 0) {
            [control removeTab:tab selected:NO animated:NO];
        }
    }
    
    NSTabViewItem * item = [self.tabView selectedTabViewItem];
    if ( ![[item identifier] isEqual:rootTabId] ) {
        if (control.tabs.count > 0) {
            if ( ![control selectedTab] ) {
                [control selectTab:control.tabs.firstObject];
            }
        } else {
            [self.tabView selectTabViewItemWithIdentifier:rootTabId];
            [control selectTab:nil];
        }
    }
}

- (void)tabs:(ASCTabsControl *)control didRequestCloseAllTabs:(ASCTabView *)tab {
    NSMutableArray<ASCTabView *> *tabsWithChanges = [NSMutableArray array];
    NSArray *tabsCopy = [NSArray arrayWithArray:control.tabs];
    for (ASCTabView *tab in tabsCopy) {
        NSCefView *cefView = [self cefViewWithTab:tab];
        if (cefView && ([cefView checkCloudCryptoNeedBuild] || [cefView checkBuilding])) {
            continue;
        }
        
        if (cefView && ([cefView.data hasChanges] || [cefView isSaveLocked])) {
            [tabsWithChanges addObject:tab];
        } else {
            [control removeTab:tab selected:NO animated:NO];
        }
    }
    
    if (tabsWithChanges.count > 0) {
        self.tabsWithChanges = tabsWithChanges;
        [self safeCloseTabsWithChanges];
    } else {
        NSTabViewItem *item = [self.tabView selectedTabViewItem];
        if (![[item identifier] isEqual:rootTabId]) {
            if (control.tabs.count > 0) {
                if (![control selectedTab]) {
                    [control selectTab:control.tabs.firstObject];
                }
            } else {
                [self.tabView selectTabViewItemWithIdentifier:rootTabId];
                [control selectTab:nil];
            }
        }
    }
}

- (void)tabs:(ASCTabsControl *)control didRequestShowInFolderForTab:(ASCTabView *)tab {
    if (tab) {
        NSString *filePath = tab.params[@"path"];
        if (filePath && filePath.length > 0) {
            NSURL *url = [NSURL fileURLWithPath:filePath];
            if (url && [[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
                [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:@[url]];
            }
        }
    }
}

- (void)tabs:(ASCTabsControl *)control didRequestDetachTab:(ASCTabView *)tab {
    if (tab) {
        NSCefView *webView = (NSCefView *)tab.webView;
        if (!webView || ![webView.data isViewType:cvwtEditor]) {
            return;
        }
        
        [webView removeFromSuperview];
        tab.webView = nil;
        tab.params[@"detached"] = @YES;
        [control removeTab:tab animated:NO];
        webView.data.url = tab.params[@"url"];
        webView.data.path = tab.params[@"path"];
        webView.data.cloudName = tab.params[@"cloud"];
                
        AppDelegate *app = [NSApp delegate];
        ASCTitleWindowController *mainWindowController = (ASCTitleWindowController *)app.mainWindowController;
        NSRect mainFrame = [mainWindowController normalFrame];
        mainFrame.origin.x += 30;
        mainFrame.origin.y -= 30;
        
        NSWindow* editorWindow = [app editorWindowFromCef:webView withFrame:mainFrame];
        if ( [mainWindowController.window isZoomed] ) {
            [editorWindow setIsZoomed:YES];
        }
    }
}

- (void)tabs:(ASCTabsControl *)control didRequestCreateNewForTab:(ASCTabView *)tab {
    if (tab) {
        ASCTabViewType type = tab.type;
        NSInteger createType = -1;
        
        switch (type) {
            case ASCTabViewTypeDocument:
                createType = (int)AscEditorType::etDocument;
                break;
            case ASCTabViewTypeSpreadsheet:
                createType = (int)AscEditorType::etSpreadsheet;
                break;
            case ASCTabViewTypePresentation:
                createType = (int)AscEditorType::etPresentation;
                break;
            case ASCTabViewTypePdf:
                createType = (int)AscEditorType::etDocumentMasterForm;
                break;
            default:
                break;
        }
        
        if (createType >= 0) {
            [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameCreateTab
                                                                object:nil
                                                              userInfo:@{
                @"action"  : @(ASCTabActionCreateLocalFile),
                @"type"    : @(createType),
                @"active"  : @(YES)
            }];
        }
    }
}

#pragma mark -
#pragma mark ASCTitleBarController Delegate

- (void)onOnlyofficeButton:(id)sender {
    [self.tabView selectTabViewItemWithIdentifier:rootTabId];
    [self.tabsControl selectTab:nil];
}

- (void)onShowUserInfoController:(NSViewController *)controller {
    ASCUserInfoViewController *userInfoController = (ASCUserInfoViewController *)controller;
    userInfoController.delegate = self;
}

#pragma mark -
#pragma mark ASCUserInfoViewController Delegate

- (void)onLogoutButton:(ASCUserInfoViewController *)controller {
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    NSDictionary * userInfo = [[ASCHelper localSettings] valueForKey:ASCUserSettingsNameUserInfo];
    
    if (userInfo) {
        NSString * portal = userInfo[@"portal"];
        
        if (portal) {
            appManager->Logout([portal stdwstring]);
            
            [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                     action:@"Portal Logout"
                                                                      label:nil
                                                                      value:nil];
        }
    }
}

#pragma mark -
#pragma mark NSTabView Delegate

- (void)tabView:(NSTabView *)tabView dimTabViewItem:(nullable NSTabViewItem *)tabViewItem {
    [self tabView:tabView dimTabViewItem:tabViewItem delay:0.33];
}

- (void)tabView:(NSTabView *)tabView dimTabViewItem:(nullable NSTabViewItem *)tabViewItem delay:(double)delay {
    if (!tabViewItem) {
        return;
    }
    
    NSImage * selectedTabViewItemImage = [[[tabView selectedTabViewItem] view] windowScreenshot];
    NSView * dimView = [[NSView alloc] initWithFrame:tabView.frame];
    dimView.backgroundColor = [NSColor windowBackgroundColor];
    
    if (selectedTabViewItemImage) {
        dimView.wantsLayer = true;
        [dimView.layer setContents:selectedTabViewItemImage];
        [dimView.layer setContentsGravity:kCAGravityResize];
    }
    
    [self.view addSubview:dimView];
            
    [dimView displayIfNeeded];
    [self.view displayIfNeeded];
            
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.001]];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delay * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        [dimView removeFromSuperview];
    });
}

- (BOOL)tabView:(NSTabView *)tabView shouldSelectTabViewItem:(nullable NSTabViewItem *)tabViewItem {
    [self tabView:tabView dimTabViewItem:tabViewItem delay:0.1];
    return true;
}

#pragma mark -
#pragma mark Debug

//- (IBAction)onAddTab:(id)sender {
//    ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
//    tab.title = [NSString stringWithFormat:@"Tab %lu", (unsigned long)rand() % 10000];
//    [self.tabsControl addTab:tab];
//}
//
//- (IBAction)onRemoveTab:(id)sender {
//    [self.tabsControl removeTab:[[self.tabsControl tabs] firstObject]];
//}
@end

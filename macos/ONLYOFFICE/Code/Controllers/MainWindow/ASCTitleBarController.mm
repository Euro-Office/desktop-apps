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
//  ASCTitleBarController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/8/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTitleBarController.h"
#import "ASCTitleWindow.h"
#import "ASCTitleWindowController.h"
#import "ASCConstants.h"
#import "NSView+Extensions.h"
#import "NSColor+Extensions.h"
#import "NSApplication+Extensions.h"
#import "ASCTabView.h"
#import "SFBPopover.h"
#import "ASCUserInfoViewController.h"
#import "ASCDownloadViewController.h"
#import "ASCDownloadController.h"
#import "ASCMenuButtonCell.h"
#import "ASCThemesController.h"
#import "ASCApplicationManager.h"
#import "AppDelegate.h"
#import "ASCLinguist.h"
#import "NSWindow+Extensions.h"
#import "NSCefView.h"
#import "NSDictionary+Extensions.h"


static float kASCWindowDefaultTrafficButtonsLeftMargin = 0;
static float kASCWindowMinTitleWidth = 0;
static float kASCRTLTabsRightMargin = 0;

@interface ASCTitleBarController ()  <ASCTabsControlDelegate, ASCDownloadControllerDelegate>
@property (nonatomic) NSArray *standardButtonsDefaults;
@property (nonatomic) NSArray *standardButtonsFullscreen;

@property (nonatomic, weak) NSButton *closeButtonFullscreen;
@property (nonatomic, weak) NSButton *miniaturizeButtonFullscreen;
@property (nonatomic, weak) NSButton *fullscreenButtonFullscreen;

@property (weak) IBOutlet NSView *titleContainerView;
@property (weak) IBOutlet NSButton *portalButton;
@property (weak) IBOutlet NSButton *userProfileButton;
@property (weak) IBOutlet NSLayoutConstraint *downloadWidthConstraint;
@property (weak) IBOutlet NSLayoutConstraint *buttonPortalLeadingConstraint;
@property (weak) IBOutlet NSLayoutConstraint *buttonPortalTrailingConstraint;
@property (weak) IBOutlet NSImageView *downloadImageView;
@property (weak) IBOutlet NSView *downloadBackgroundView;
@property (nonatomic) SFBPopover * popover;
@end

@implementation ASCTitleBarController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self initialize];
}

- (void)initialize {
    NSArray * windows = [[NSApplication sharedApplication] windows];
    NSWindow * mainWindow = nil;
    
    for (NSWindow * window in windows) {
        if ([window isKindOfClass:[ASCTitleWindow class]]) {
            mainWindow = window;
            break;
        }
    }

    // Standart window buttons in Fullscreen

    self.closeButtonFullscreen = [NSWindow standardWindowButton:NSWindowCloseButton forStyleMask:NSWindowStyleMaskTitled];
    self.fullscreenButtonFullscreen = [NSWindow standardWindowButton:NSWindowZoomButton forStyleMask:NSWindowStyleMaskTitled];
    self.miniaturizeButtonFullscreen = [NSWindow standardWindowButton:NSWindowMiniaturizeButton forStyleMask:NSWindowStyleMaskTitled];
    self.miniaturizeButtonFullscreen.enabled = NO;

    self.standardButtonsFullscreen = @[self.closeButtonFullscreen, self.miniaturizeButtonFullscreen, self.fullscreenButtonFullscreen];
    if ( [self.view userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft )
        self.standardButtonsFullscreen = [[self.standardButtonsFullscreen reverseObjectEnumerator] allObjects];

    [self.standardButtonsFullscreen enumerateObjectsUsingBlock:^(NSView *standardButtonView, NSUInteger idx, BOOL *stop) {
        [self.view addSubview:standardButtonView];
    }];

    // Standart window buttons

    if (mainWindow) {
        self.standardButtonsDefaults = @[[mainWindow standardWindowButton:NSWindowCloseButton],
                                         [mainWindow standardWindowButton:NSWindowMiniaturizeButton],
                                         [mainWindow standardWindowButton:NSWindowZoomButton]];
        
        if ( [self.view userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft ) {
            self.standardButtonsDefaults = [[self.standardButtonsDefaults reverseObjectEnumerator] allObjects];
        }
    }
    
    [self.standardButtonsDefaults enumerateObjectsUsingBlock:^(NSButton *standardButton, NSUInteger idx, BOOL *stop) {
        [self.view addSubview:standardButton];
    }];

    // Other window controls

    self.downloadWidthConstraint.constant = .0f;
    self.downloadImageView.canDrawSubviewsIntoLayer = YES;
    self.downloadImageView.animates = NO;
    self.downloadImageView.hidden = NO;

    NSDataAsset * asset = [[NSDataAsset alloc] initWithName:@"progress_download_icon"];
    NSBitmapImageRep * rep = [[NSBitmapImageRep alloc] initWithData:[asset data]];
//    int framescount = [[rep valueForProperty:NSImageFrameCount] intValue];
    [rep setProperty:NSImageCurrentFrame withValue:[NSNumber numberWithUnsignedInt:15]];
    NSData *repData = [rep representationUsingType:NSPNGFileType properties:nil];
    NSImage * img = [[NSImage alloc] initWithData:repData];
    [self.downloadImageView setImage:img];

//    NSImage * image = [NSImage imageNamed:@"progress_download_icon"];
//    if ( image ) {
//
//        NSLog(@"download1: image loaded, %d", framescount);
////        self.downloadImageView.image = image;
//        [self.downloadImageView setImage:img];
//    } else {
//        NSLog(@"download1: load failed");
//    }

    kASCWindowDefaultTrafficButtonsLeftMargin = NSWidth(self.closeButtonFullscreen.frame) - 2.0; // OSX 10.11 magic

    // Subscribe to events

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowDidResize:)
                                                 name:NSWindowDidResizeNotification
                                               object:mainWindow];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onWindowSetFrame:)
                                                 name:ASCEventNameMainWindowSetFrame
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFChangedTabEditorType:)
                                                 name:CEFEventNameTabEditorType
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFChangedTabEditorName:)
                                                 name:CEFEventNameTabEditorNameChanged
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFModifyChanged:)
                                                 name:CEFEventNameModifyChanged
                                               object:nil];
       
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onCEFLogin:)
                                                 name:CEFEventNameLogin
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onChangedUITheme:)
                                                 name:ASCEventNameChangedUITheme
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onChangedSystemTheme:)
                                                 name:ASCEventNameChangedSystemTheme
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowDidBecomeKey:)
                                                 name:NSWindowDidBecomeKeyNotification
                                               object:mainWindow];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowDidResignKey:)
                                                 name:NSWindowDidResignKeyNotification
                                               object:mainWindow];

    [[[ASCDownloadController sharedInstance] multicastDelegate] addDelegate:self];
    [self.tabsControl.multicastDelegate addDelegate:self];
    
//    [self.userProfileButton setHidden:YES];
        [self.userProfileButton setHidden:NO];
    [self.portalButton setState:NSControlStateValueOn];
    if ( [self.view userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft ) {
        self.buttonPortalLeadingConstraint.constant = -1;
        self.buttonPortalTrailingConstraint.constant = 0;

        kASCRTLTabsRightMargin = CGRectGetMinX([[self.tabsControl superview] frame]);
    }
    
    [self.tabsControl removeAllConstraints];

    ASCMenuButtonCell * portalButtonCell = self.portalButton.cell;

    if (portalButtonCell) {
        if (@available(macOS 10.13, *)) {
            portalButtonCell.bgColor            = [NSColor colorNamed:@"tab-inactiveColor"];
            portalButtonCell.bgHoverColor       = [NSColor colorNamed:@"tab-hoverInactiveColor"];
            portalButtonCell.bgActiveColor      = [ASCThemesController currentThemeColor:btnPortalActiveBackgroundColor];
            portalButtonCell.textColor          = [NSColor clearColor];
            portalButtonCell.textActiveColor    = [NSColor clearColor];
            portalButtonCell.lineColor          = [NSColor clearColor];
        } else {
            portalButtonCell.bgColor            = kColorRGBA(255, 255, 255, 0.0);
            portalButtonCell.bgHoverColor       = kColorRGBA(255, 255, 255, 1.0);
            portalButtonCell.bgActiveColor      = kColorRGBA(255, 255, 255, 1.0);
            portalButtonCell.textColor          = kColorRGBA(255, 255, 255, 0.0);
            portalButtonCell.textActiveColor    = kColorRGBA(255, 255, 255, 0.0);
            portalButtonCell.lineColor          = kColorRGBA(255, 255, 255, 0.0);
        }

        if ( [ASCThemesController isCurrentThemeDark] ) {
            [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-light"]];
        }
    }


    [self doLayout];
}

- (void)windowDidResize:(NSNotification *)notification {
    [self doLayout];
}

- (void)onWindowSetFrame:(NSNotification *)notification {
    [self doLayout];
}

- (BOOL)isFullScreen {
    NSArray * windows = [[NSApplication sharedApplication] windows];
    NSWindow * mainWindow = [[[NSApplication sharedApplication] windows] firstObject];
    
    for (NSWindow * window in windows) {
        if ([window isKindOfClass:[ASCTitleWindow class]]) {
            mainWindow = window;
            break;
        }
    }
    
	return (([mainWindow styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask);
}

- (void)doLayout {
    NSWindow * mainWindow = [NSWindow titleWindowOrMain];

    // Layout title and tabs
    CGFloat containerWidth  = CGRectGetWidth(self.titleContainerView.frame);
    CGFloat maxTabsWidth    = containerWidth - kASCWindowMinTitleWidth - 100;
    CGFloat actualTabsWidth = self.tabsControl.maxTabWidth * [self.tabsControl.tabs count];

    int btnSpacing = 6.0;
    CGFloat btnContainerWidth = CGRectGetWidth([self.standardButtonsDefaults[0] frame]) + btnSpacing;
    CGFloat leftOffsetForTrafficLightButtons = kASCWindowDefaultTrafficButtonsLeftMargin;
    int rtlDependedTabsLeftOffset = 0;
    if ( [ASCLinguist isUILayoutDirectionRtl] ) {
        CGFloat windowWidth = CGRectGetWidth([mainWindow frame]);
        leftOffsetForTrafficLightButtons = windowWidth - kASCWindowDefaultTrafficButtonsLeftMargin - btnContainerWidth * 3 + btnSpacing;

        NSRect rect = [[self.tabsControl superview] frame];
        rtlDependedTabsLeftOffset = windowWidth - kASCRTLTabsRightMargin - CGRectGetMinX(rect) - MIN(actualTabsWidth, maxTabsWidth);
    }

    void (^layoutStandartButtons)(NSArray *, BOOL) = ^ (NSArray *views, BOOL hidden) {
        [views enumerateObjectsUsingBlock:^(NSView *view, NSUInteger idx, BOOL *stop) {
            NSRect frame = view.frame;
            frame.origin.x = leftOffsetForTrafficLightButtons + idx * btnContainerWidth;
            frame.origin.y = (int)((NSHeight(view.superview.frame) - NSHeight(view.frame)) / 2.0);
            
            [view setFrame:frame];
            [view setHidden:hidden];
            [view setNeedsDisplay:YES];
        }];
    };

    layoutStandartButtons(self.standardButtonsDefaults, [self isFullScreen]);
    layoutStandartButtons(self.standardButtonsFullscreen, ![self isFullScreen]);

    self.tabsControl.frame  = CGRectMake(rtlDependedTabsLeftOffset, 0, MIN(actualTabsWidth, maxTabsWidth), CGRectGetHeight(self.tabsControl.frame));
}

- (void)viewWillTransitionToSize:(NSSize)newSize {
    [self doLayout];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    [self updateWindowButtonsAppearance];
}

- (void)windowDidResignKey:(NSNotification *)notification {
    [self updateWindowButtonsAppearance];
}

- (void)updateWindowButtonsAppearance {
    [self.standardButtonsFullscreen enumerateObjectsUsingBlock:^(NSView *view, NSUInteger idx, BOOL *stop) {
        [view setNeedsDisplay:YES];
    }];
    [self.standardButtonsDefaults enumerateObjectsUsingBlock:^(NSView *view, NSUInteger idx, BOOL *stop) {
        [view setNeedsDisplay:YES];
    }];
}

#pragma mark -
#pragma mark CEF events handler
- (void)onCEFChangedTabEditorType:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        NSInteger type          = [params[@"type"] integerValue];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
//        NSLog(@"on change editor type %ld %lu", type);
        
        if (tab) {
            ASCTabViewType docType = ASCTabViewTypeUnknown;
            switch (AscEditorType(type)) {
                case AscEditorType::etDocument     : docType = ASCTabViewTypeDocument; break;
                case AscEditorType::etSpreadsheet  : docType = ASCTabViewTypeSpreadsheet; break;
                case AscEditorType::etPresentation : docType = ASCTabViewTypePresentation; break;
                case AscEditorType::etPdf          : docType = ASCTabViewTypePdf; break;
                case AscEditorType::etDraw         : docType = ASCTabViewTypeDraw; break;
                default:
                    break;
            }

            if (NSCefView *cefView = (NSCefView *)tab.webView) {
                [cefView.data setContentType:AscEditorType(type)];
            }
            [tab setType:docType];
            [self.tabsControl updateTab:tab];
        }
    }
}

- (void)onCEFChangedTabEditorName:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        NSString * name         = params[@"name"];
        NSString * path         = params[@"path"];

        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];
        
        if (tab) {
            if (NSCefView *cefView = (NSCefView *)tab.webView) {
                [cefView.data setTitle:name];
            }
            [tab setTitle:name];
            [tab setToolTip:name];

            if ( !(path == nil) && !(path.length == 0) ) {
                tab.params[@"path"] = path;
            }

            [self.tabsControl updateTab:tab];
//            if ([tab state] == NSControlStateValueOn) {
//                [self.tabsControl reloadTab:tab];
//            }
        }
    }
}

- (void)onCEFModifyChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params   = (NSDictionary *)notification.userInfo;
        NSString * viewId       = params[@"viewId"];
        BOOL changed            = [params[@"сhanged"] boolValue];
        
        ASCTabView * tab = [self.tabsControl tabWithUUID:viewId];

        if (tab) {
            if (NSCefView *cefView = (NSCefView *)tab.webView) {
                [cefView.data setChanged:changed];
                
                [tab setTitle:[cefView.data title:NO]];
                [self.tabsControl updateTab:tab];
            }
            
//            if ([tab state] == NSControlStateValueOn) {
//                [self.tabsControl reloadTab:tab];
//            }
        }
    }
}

- (void)onCEFLogin:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * userInfo = (NSDictionary *)notification.userInfo;
        
        [[ASCHelper localSettings] setValue:userInfo forKey:ASCUserSettingsNameUserInfo];
        [self.userProfileButton setHidden:NO];
    }
}

-(void)onChangedUITheme:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * theme = params[@"uitheme"];

        if ( [theme isEqualToString: uiThemeSystem] ) {
            theme = [ASCThemesController defaultThemeId:[NSApplication isSystemDarkMode]];
        }

        ASCMenuButtonCell * portalButtonCell = self.portalButton.cell;
        portalButtonCell.bgActiveColor = [ASCThemesController color:btnPortalActiveBackgroundColor forTheme:theme];
        [self.portalButton setNeedsDisplay];

        if ( [self.portalButton state] != NSControlStateValueOn ) {
            [ASCThemesController isDarkWindowAppearance] ? [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-light"]] :
                                                    [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-dark"]];
        } else {
            [ASCThemesController isCurrentThemeDark] ? [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-light"]] :
                                                    [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-dark"]];
        }

        for (ASCTabView * tab in self.tabsControl.tabs) {
            if ( [tab state] == NSControlStateValueOn ) {
                [tab setNeedsDisplay];
            }
            [tab setType:tab.type];
            [self.tabsControl updateTab:tab];
        }
    }
}

- (void)onChangedSystemTheme:(NSNotification *)notification {
    if ( [[ASCThemesController currentThemeId] isEqualToString: uiThemeSystem] ) {
        if (notification && notification.userInfo) {
            NSString * theme = [ASCThemesController defaultThemeId:[ASCThemesController isSystemDarkMode]];

            ASCMenuButtonCell * portalButtonCell = self.portalButton.cell;
            portalButtonCell.bgActiveColor = [ASCThemesController color:btnPortalActiveBackgroundColor forTheme:theme];
            [self.portalButton setNeedsDisplay];

            [ASCThemesController isDarkWindowAppearance] ? [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-light"]] :
                                                        [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-dark"]];
        }
    } else {
        if ( [self.portalButton state] != NSControlStateValueOn ) {
            [ASCThemesController isDarkWindowAppearance] ? [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-light"]] :
                                                        [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-dark"]];
        }
    }

    for (ASCTabView * tab in self.tabsControl.tabs) {
        if ( [tab state] == NSControlStateValueOn ) {
            [tab setNeedsDisplay];
        }
        [tab setType:tab.type];
        [self.tabsControl updateTab:tab];
    }
}

#pragma mark -
#pragma mark Actions
- (void)setupCustomPopover:(SFBPopover *)popover {
    popover.closesWhenPopoverResignsKey     = YES;
    popover.closesWhenApplicationBecomesInactive = YES;
    popover.drawRoundCornerBesideArrow      = YES;
    popover.borderColor                     = [NSColor clearColor];
    popover.backgroundColor                 = [NSColor whiteColor];
    popover.viewMargin                      = 0.0f;
    popover.borderWidth                     = 0.0f;
    popover.cornerRadius                    = 2.0f;
    popover.drawsArrow                      = YES;
    popover.movable                         = NO;
    popover.arrowWidth                      = 20.0f;
    popover.arrowHeight                     = 10.0f;
    popover.distance                        = 10.0f;
}

- (IBAction)onOnlyofficeButton:(id)sender {
    if (_delegate && [_delegate respondsToSelector:@selector(onOnlyofficeButton:)]) {
        [_delegate onOnlyofficeButton:sender];
    }
}

- (IBAction)onUserInfoClick:(id)sender {
    ASCUserInfoViewController * controller = [self.storyboard instantiateControllerWithIdentifier:@"ASCUserInfoControllerId"];
    self.popover = [[SFBPopover alloc] initWithContentViewController:controller];
    self.popover.arrowOffset = 8.0f;
    [self setupCustomPopover:self.popover];
    
    NSRect rectOfSender = [sender convertRect:[sender bounds] toView:nil];
    NSPoint where = rectOfSender.origin;
    where.x += rectOfSender.size.width / 2;

    [controller setPopover:self.popover];
    [self.popover displayPopoverInWindow:[sender window] atPoint:where chooseBestLocation:YES];
    
    if (_delegate && [_delegate respondsToSelector:@selector(onShowUserInfoController:)]) {
        [_delegate onShowUserInfoController:controller];
    }
}

- (IBAction)onDownloadButton:(id)sender {
    ASCDownloadViewController * controller = [self.storyboard instantiateControllerWithIdentifier:@"ASCDownloadListControllerId"];
    self.popover = [[SFBPopover alloc] initWithContentViewController:controller];
    self.popover.arrowOffset = 40.0f;
    [self setupCustomPopover:self.popover];
    
    NSRect rectOfSender = [sender convertRect:[sender bounds] toView:nil];
    NSPoint where = rectOfSender.origin;
    where.x += rectOfSender.size.width / 2;
    
    [controller setPopover:self.popover];
    [self.popover displayPopoverInWindow:[sender window] atPoint:where chooseBestLocation:YES];
}

- (IBAction)onTestButton:(NSButton *)sender {
    NSStoryboard * storyboard = [NSStoryboard storyboardWithName:StoryboardNameSign bundle:[NSBundle mainBundle]];

    if (storyboard) {
        NSWindowController * windowController = [storyboard instantiateControllerWithIdentifier:@"DocSignWindowController"];
        NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];

        [mainWindow beginSheet:[windowController window] completionHandler:^(NSModalResponse returnCode) {
            //
        }];
    }
}

#pragma mark -
#pragma mark ASCTabsControl Delegate

- (void)tabs:(ASCTabsControl *)control didResize:(CGRect)rect {
    [self doLayout];
}

- (void)tabs:(ASCTabsControl *)control didSelectTab:(ASCTabView *)tab {  
    if (tab) {
        [self.portalButton setState:NSControlStateValueOff];

        if ( [ASCThemesController isDarkWindowAppearance] )
            [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-light"]];
        else [self.portalButton setImage:[NSImage imageNamed:@"logo-tab-dark"]];
    } else {
        [self.portalButton setState:NSControlStateValueOn];
        [self.portalButton setImage:[NSImage imageNamed:[ASCThemesController isCurrentThemeDark] ? @"logo-tab-light" : @"logo-tab-dark"]];
    }
}

- (BOOL)tabs:(ASCTabsControl *)control didDetachTab:(ASCTabView *)tab atScreenPoint:(NSPoint)screenPoint withEvent:(NSEvent *)event {
    NSCefView *webView = (NSCefView *)tab.webView;
    if (!webView || ![webView.data isViewType:cvwtEditor]) {
        return NO;
    }
    
    [webView removeFromSuperview];
    tab.webView = nil;
    tab.params[@"detached"] = @YES;
    [control removeTab:tab animated:NO];
    webView.data.url = tab.params[@"url"];
    webView.data.path = tab.params[@"path"];
    webView.data.cloudName = tab.params[@"cloud"];
    
    AppDelegate *app = [NSApp delegate];
    [app dragDetachedTab:webView atScreenPoint:screenPoint withEvent:event];
    
    return YES;
}

#pragma mark -
#pragma mark ASCDownloadController Delegate

- (void)downloadController:(ASCDownloadController *)controler didAddDownload:(id)download {
    self.downloadWidthConstraint.constant = ([[controler downloads] count] > 0) ? 30.f : .0f;
}

- (void)downloadController:(ASCDownloadController *)controler didRemovedDownload:(id)download {
    self.downloadWidthConstraint.constant = ([[controler downloads] count] > 0) ? 30.f : .0f;
}

- (void)downloadController:(ASCDownloadController *)controler didUpdatedDownload:(id)download {
    //
}

#pragma mark -
#pragma mark Navigation

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"ASCUserInfoSegueID"]) {
        if (_delegate && [_delegate respondsToSelector:@selector(onShowUserInfoController:)]) {
            [_delegate onShowUserInfoController:segue.destinationController];
        }
    }
}

#pragma mark -
#pragma mark Tab Attachment Support

- (void)attachEditor:(NSView *)cefView atScreenPoint:(NSPoint)screenPoint {
    NSCefView *webView =  (NSCefView *)cefView;
    [webView removeFromSuperview];
    
    NSDictionary *widgetInfo = @{@"widgetType": @"tab", @"captionHeight": @0};
    [webView setParentWidgetInfoWithJson:[widgetInfo jsonString]];
    
    ASCTabViewType docType = ASCTabViewTypeUnknown;
    switch ([webView.data contentType]) {
        case AscEditorType::etDocument     : docType = ASCTabViewTypeDocument; break;
        case AscEditorType::etSpreadsheet  : docType = ASCTabViewTypeSpreadsheet; break;
        case AscEditorType::etPresentation : docType = ASCTabViewTypePresentation; break;
        case AscEditorType::etPdf          : docType = ASCTabViewTypePdf; break;
        case AscEditorType::etDraw         : docType = ASCTabViewTypeDraw; break;
        default:
            break;
    }
    
    ASCTabView *tab = [[ASCTabView alloc] initWithFrame:CGRectZero];
    tab.title       = [webView.data title:NO];
    tab.type        = docType;
    tab.webView = webView;
    tab.params = [NSMutableDictionary dictionary];
    tab.params[@"action"] = @(ASCTabActionUnknown);
    tab.params[@"url"] = webView.data.url;
    tab.params[@"path"] = webView.data.path;
    tab.params[@"reattaching"] = @YES;
    
    NSInteger index = [self insertionIndexForScreenPoint:screenPoint];
    [self.tabsControl insertTab:tab atIndex:index selected:YES];
    
    NSDictionary *windowFeatures = @{@"skiptoparea": @0, @"singlewindow": @NO};
    [webView sendCommand:@"window:features" withParam:[windowFeatures jsonString]];
    
    [webView focus];
}

- (NSInteger)insertionIndexForScreenPoint:(NSPoint)screenPoint {
    NSWindow *window = self.view.window;
    NSPoint windowPoint;
    if (@available(macOS 10.12, *)) {
        windowPoint = [window convertPointFromScreen:screenPoint];
    } else {
        NSRect screenRect = NSMakeRect(screenPoint.x, screenPoint.y, 0, 0);
        NSRect windowRect = [window convertRectFromScreen:screenRect];
        windowPoint = windowRect.origin;
    }
    
    NSPoint pointInTabs = [self.tabsControl convertPoint:windowPoint fromView:nil];

    NSInteger tabsCount = [self.tabsControl.tabs count];
    if (tabsCount == 0) {
        return 0;
    }

    for (NSInteger i = 0; i < tabsCount; ++i) {
        ASCTabView *tab = [self.tabsControl.tabs objectAtIndex:i];
        NSRect tabFrameInTabs = [tab convertRect:tab.bounds toView:self.tabsControl];
        CGFloat midX = NSMidX(tabFrameInTabs);
        if (pointInTabs.x < midX) {
            return i;
        }
    }

    return tabsCount;
}

- (BOOL)canPinTabAtPoint:(NSPoint)screenPoint {
    NSWindow * mainWindow = self.view.window;
    NSRect windowFrame = mainWindow.frame;
    
    BOOL containsPoint = NO;
    
    if (NSPointInRect(screenPoint, windowFrame)) {
        if ( [self isFullScreen] ) {
            const CGFloat dropZoneHeight = 36.0;
            NSRect dropZoneRect = NSMakeRect(windowFrame.origin.x, windowFrame.origin.y + windowFrame.size.height - dropZoneHeight,
                                             windowFrame.size.width, dropZoneHeight);
            containsPoint = NSPointInRect(screenPoint, dropZoneRect);
        } else {
            NSRect contentRect = [mainWindow contentRectForFrameRect:windowFrame];
            NSRect titleBarRect = NSMakeRect(windowFrame.origin.x, contentRect.origin.y + contentRect.size.height,
                                             windowFrame.size.width, windowFrame.size.height - contentRect.size.height);
            containsPoint = NSPointInRect(screenPoint, titleBarRect);
        }
        
        if (containsPoint) {
            NSRect portalButtonRect = [self.portalButton convertRect:self.portalButton.bounds toView:nil];
            portalButtonRect = [mainWindow convertRectToScreen:portalButtonRect];
            if (NSMidX(portalButtonRect) < NSMidX(windowFrame)
                    ? screenPoint.x <= NSMaxX(portalButtonRect)
                    : screenPoint.x >= NSMinX(portalButtonRect)) {
                containsPoint = NO;
            }
        }
    }
    
    BOOL pinAllowed = self.tabsControl.tabPinAllowed;
    if (!containsPoint && !pinAllowed) {
        self.tabsControl.tabPinAllowed = YES;
        return NO;
    }
    
    return containsPoint && pinAllowed;
}

@end

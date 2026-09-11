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
//  ASCTouchBarController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 02/01/2019.
//  Copyright (c) 2019 Ascensio System SIA. All rights reserved.
//
#import <Quartz/Quartz.h>
#import "ASCTouchBarController.h"
#import "ASCTabTouchBarItem.h"
#import "ASCBlockHolder.h"
#import "NSColor+Extensions.h"
#import "ASCTabsControl.h"
#import "ASCTabView.h"
#import "ASCHelper.h"

static NSTouchBarCustomizationIdentifier const kScrubberCustomizationIdentifier = @"com.onlyoffice.touchbar.scrubberViewController";
static NSTouchBarItemIdentifier const kScrubbedItemIdentifier = @"com.onlyoffice.touchbar.item.scrubber";
static NSTouchBarItemIdentifier const kStartPageItemIdentifier = @"com.onlyoffice.touchbar.item.startpage";
static NSTouchBarItemIdentifier const kNewItemsItemIdentifier = @"com.onlyoffice.touchbar.item.newItems";

@interface ASCTouchBarController () <NSTouchBarDelegate, NSScrubberDelegate, NSScrubberDataSource, NSScrubberFlowLayoutDelegate, ASCTabsControlDelegate>
@property (weak) NSViewController *viewController;
@property (nonatomic) NSScrubber *tabsScrubber;
@property (nonatomic) NSButton *startPageButton;
@end

@implementation ASCTouchBarController

- (instancetype)init:(NSViewController *)viewController {
    self = [super init];
    if (self) {
        _tabs = [NSMutableArray array];
        _viewController = viewController;
        _selectedIndex = -1;
        [self initDemoData];
    }
    return self;
}

- (void)initDemoData {
    //
}

- (void)setSelectedIndex:(NSInteger)selectedIndex {
    _selectedIndex = selectedIndex;

    self.startPageButton.bezelColor = _selectedIndex < 0 ? [NSColor grayColor] : nil;
    [self.startPageButton setNeedsDisplay];
    [self.startPageButton setNeedsLayout:YES];

    if (self.tabsScrubber) {
        self.tabsScrubber.selectedIndex = selectedIndex;

        if (selectedIndex > -1) {
            [[self.tabsScrubber animator] scrollItemAtIndex:selectedIndex toAlignment:NSScrubberAlignmentNone];
        }
    }
}

- (NSCustomTouchBarItem *)makeButtonWithIdentifier:(NSString *)theIdentifier
                                             color:(NSColor *)color
                                             title:(NSString *)title
                                             image:(NSImage *)image
                                customizationLabel:(NSString *)customizationLabel
{
    NSButton * button;
    
    ASCBlockHolder * blockHolder = [[ASCBlockHolder alloc] initWithBlock:^{
        if (self.onItemTap) {
            self.onItemTap(button, theIdentifier);
        }
    }];
    
    button = [NSButton buttonWithTitle:title image:image target:blockHolder action:@selector(invoke:)];
    button.bezelColor = color;
    NSCustomTouchBarItem *touchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:theIdentifier];
    touchBarItem.view = button;
    touchBarItem.customizationLabel = customizationLabel;
    
    return touchBarItem;
}

#pragma mark -
#pragma mark NSTouchBar

// Used to invalidate the current NSTouchBar.
- (void)invalidateTouchBar
{
    if (@available(macOS 10.12.2, *)) {
        if (_viewController == nil) {
            return;
        }

        self.tabsScrubber = nil;

        // We need to set the first responder status when one of our radio knobs was clicked.
        [self.viewController.view.window makeFirstResponder:self.viewController.view];

        // Set to nil so makeTouchBar can be called again to re-create our NSTouchBarItem instances.
        self.viewController.touchBar = nil;
    }
}

- (NSTouchBar *)makeTouchBar
{
    if (@available(macOS 10.12.2, *)) {
        NSTouchBar *bar = [[NSTouchBar alloc] init];
        bar.delegate = self;

        bar.customizationIdentifier = kScrubberCustomizationIdentifier;

        // Set the default ordering of items.
        NSArray * items = !(_tabs.count > 0) ? @[kNewItemsItemIdentifier, NSTouchBarItemIdentifierOtherItemsProxy, NSTouchBarItemIdentifierFlexibleSpace] :
                                @[kStartPageItemIdentifier, kScrubbedItemIdentifier, NSTouchBarItemIdentifierOtherItemsProxy];

        if ([self.viewController.view userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft) {
            items = [[items reverseObjectEnumerator] allObjects];
        }
        bar.defaultItemIdentifiers = items;

        //    bar.customizationAllowedItemIdentifiers = @[kScrubbedItemIdentifier];
        //    bar.principalItemIdentifier = kScrubbedItemIdentifier;

        return bar;
    }

    return nil;
}

- (NSInteger)indexOf:(ASCTabView *)tab {
    __block ASCTabView * tabView = tab;
    return [self.tabs indexOfObjectPassingTest:^BOOL(ASCTabTouchBar * _Nonnull touchBarTab, NSUInteger idx, BOOL * _Nonnull stop) {
        if ([touchBarTab.uuid isEqualToString:tabView.uuid]) {
            *stop = YES;
            return YES;
        }
        return NO;
    }];
}

- (BOOL)updateTabInfoFor:(ASCTabView *)tab {
    BOOL isChange = NO;
    NSUInteger index = [self indexOf:tab];

    if (index != NSNotFound) {
        switch (tab.type) {
            case ASCTabViewTypeDocument:
                isChange = self.tabs[index].type != ASCTabTouchBarTypeDocument;
                self.tabs[index].type = ASCTabTouchBarTypeDocument;
                break;
            case ASCTabViewTypeSpreadsheet:
                isChange = self.tabs[index].type != ASCTabTouchBarTypeSpreadsheet;
                self.tabs[index].type = ASCTabTouchBarTypeSpreadsheet;
                break;
            case ASCTabViewTypePresentation:
                isChange = self.tabs[index].type != ASCTabTouchBarTypePresentation;
                self.tabs[index].type = ASCTabTouchBarTypePresentation;
                break;
            case ASCTabViewTypePdf:
                isChange = self.tabs[index].type != ASCTabTouchBarTypePdf;
                self.tabs[index].type = ASCTabTouchBarTypePdf;
                break;
            default:
                isChange = self.tabs[index].type != ASCTabTouchBarTypePortal;
                self.tabs[index].type = ASCTabTouchBarTypePortal;
                break;
        }
        isChange = isChange || ![self.tabs[index].title isEqualToString:tab.title];
        self.tabs[index].title = tab.title;
    }

    return isChange;
}

#pragma mark -
#pragma mark NSScrubberDataSource

NSString *tabScrubberItemIdentifier = @"tabItem";

- (NSInteger)numberOfItemsForScrubber:(NSScrubber *)scrubber {
    return _tabs.count;
}

- (NSScrubberItemView *)scrubber:(NSScrubber *)scrubber viewForItemAtIndex:(NSInteger)index {
    ASCTabTouchBarItem *tabItemView = [scrubber makeItemWithIdentifier:tabScrubberItemIdentifier owner:nil];
    
    if (index < _tabs.count) {
        tabItemView.tabInfo = [_tabs objectAtIndex:index];
    }
    
    return tabItemView;
}

#pragma mark -
#pragma mark NSScrubberFlowLayoutDelegate

- (NSSize)scrubber:(NSScrubber *)scrubber layout:(NSScrubberFlowLayout *)layout sizeForItemAtIndex:(NSInteger)itemIndex {
    return NSMakeSize(160, 30);
}

#pragma mark -
#pragma mark NSScrubberDelegate

- (void)scrubber:(NSScrubber *)scrubber didSelectItemAtIndex:(NSInteger)selectedIndex {
//    NSLog(@"selectedIndex = %ld", selectedIndex);
    if (self.onItemTap) {
        ASCTabTouchBar * tabInfo = [_tabs objectAtIndex:selectedIndex];
        self.onItemTap(tabInfo, [tabInfo uuid]);
    }
}

- (nullable NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier {
     // Create the Start Page tabs
    if ([identifier isEqualToString:kStartPageItemIdentifier]) {
        NSCustomTouchBarItem * startPageItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:kStartPageItemIdentifier];
        ASCBlockHolder * blockHolder = [[ASCBlockHolder alloc] initWithBlock:^{
            if (self.onItemTap) {
                self.onItemTap(self.startPageButton, kStartPageButtonIdentifier);
            }
        }];
        _startPageButton = [NSButton buttonWithTitle:[ASCHelper appNameShort]
                                               image:[NSImage imageNamed:@"touchbar-tab-startpage"]
                                              target:blockHolder
                                              action:@selector(invoke:)];
        startPageItem.view = _startPageButton;
        [startPageItem.view setUserInterfaceLayoutDirection:NSUserInterfaceLayoutDirectionLeftToRight];

        return startPageItem;
    }
    // Createtion buttons
    else if ([identifier isEqualToString:kNewItemsItemIdentifier]) {
        NSArray * creationButtons = @[
                                      [self makeButtonWithIdentifier:[NSString stringWithFormat:kCreationButtonIdentifier, @"document"]
                                                               // color:[NSColor themedDocumentEditor]
                                                               color:[NSColor brendDocumentEditor]
                                                               title:NSLocalizedStringWithDefaultValue(@"new-document", @"Localizable", [NSBundle mainBundle], @"New Document", nil)
                                                               image:[NSImage imageNamed:NSImageNameTouchBarAddDetailTemplate]
                                                  customizationLabel:NSLocalizedStringWithDefaultValue(@"new-document", @"Localizable", [NSBundle mainBundle], @"New Document", nil)],
                                      [self makeButtonWithIdentifier:[NSString stringWithFormat:kCreationButtonIdentifier, @"spreadsheet"]
                                                               color:[NSColor brendSpreadsheetEditor]
                                                               title:NSLocalizedStringWithDefaultValue(@"new-spreadsheet", @"Localizable", [NSBundle mainBundle], @"New Spreadsheet", nil)
                                                               image:[NSImage imageNamed:NSImageNameTouchBarAddDetailTemplate]
                                                  customizationLabel:NSLocalizedStringWithDefaultValue(@"new-spreadsheet", @"Localizable", [NSBundle mainBundle], @"New Spreadsheet", nil)],
                                      [self makeButtonWithIdentifier:[NSString stringWithFormat:kCreationButtonIdentifier, @"presentation"]
                                                               color:[NSColor brendPresentationEditor]
                                                               title:NSLocalizedStringWithDefaultValue(@"new-presentation", @"Localizable", [NSBundle mainBundle], @"New Presentation", nil)
                                                               image:[NSImage imageNamed:NSImageNameTouchBarAddDetailTemplate]
                                                  customizationLabel:NSLocalizedStringWithDefaultValue(@"new-presentation", @"Localizable", [NSBundle mainBundle], @"New Presentation", nil)],
                                      [self makeButtonWithIdentifier:[NSString stringWithFormat:kCreationButtonIdentifier, @"pdfform"]
                                                               color:[NSColor brandPdfEditor]
                                                               title:NSLocalizedStringWithDefaultValue(@"new-pdfform", @"Localizable", [NSBundle mainBundle], @"New PDF Form", nil)
                                                               image:[NSImage imageNamed:NSImageNameTouchBarAddDetailTemplate]
                                                  customizationLabel:NSLocalizedStringWithDefaultValue(@"new-pdfform", @"Localizable", [NSBundle mainBundle], @"New PDF Form", nil)],
                                      ];
        
        if ([self.viewController.view userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft)
            creationButtons = [[creationButtons reverseObjectEnumerator] allObjects];
        NSGroupTouchBarItem * createonGroup = [NSGroupTouchBarItem groupItemWithIdentifier:kNewItemsItemIdentifier items:creationButtons];
        return createonGroup;
    }
    // Create the tabs of open documents
    else if ([identifier isEqualToString:kScrubbedItemIdentifier]) {
        // Create the scrubber that uses tabs.
        NSCustomTouchBarItem * scrubberItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:kScrubbedItemIdentifier];
        _tabsScrubber = [[NSScrubber alloc] initWithFrame:NSMakeRect(0, 0, 310, 30)];
        NSScrubber * scrubber = _tabsScrubber;
        
        scrubber.delegate = self;   // This is so we can respond to selection.
        scrubber.dataSource = self; // This is so we can determine the content.

        // Scrubber will use just tabs.
        [scrubber registerClass:[ASCTabTouchBarItem class] forItemIdentifier:tabScrubberItemIdentifier];
        
        // For the image scrubber, we want the control to draw a fade effect to indicate that there is additional unscrolled content.
        scrubber.showsAdditionalContentIndicators = YES;
        scrubber.selectedIndex = self.selectedIndex; // Always select the first item in the scrubber.
        
        // Layout
        NSScrubberFlowLayout *layout = [NSScrubberFlowLayout new];
        layout.itemSpacing = 8;
        scrubber.scrubberLayout = layout;
        
//        scrubber.backgroundColor = [NSColor redColor];

        // Note: You can make the text-based scrubber's background transparent by using:
        // scrubber.backgroundColor = [NSColor clearColor];
        scrubber.mode = NSScrubberModeFree;
        
        // Provides leading and trailing arrow buttons.
        // Tapping an arrow button moves the selection index by one element; pressing and holding repeatedly moves the selection.
        //
//        scrubber.showsArrowButtons = self.showsArrows.state;
        
//        // Specify the style of decoration to place behind items that are selected and/or highlighted.
//        NSScrubberSelectionStyle *outlineStyle = [NSScrubberSelectionStyle outlineOverlayStyle];
//        scrubber.selectionBackgroundStyle = outlineStyle;
        
        
        // Specify the style of decoration to place above items that are selected and/or highlighted.
        NSScrubberSelectionStyle *outlineStyle = [NSScrubberSelectionStyle outlineOverlayStyle];
        scrubber.selectionOverlayStyle = outlineStyle;
        
        // Set the layout constraints on this scrubber so that it's 400 pixels wide.
//        NSDictionary *items = NSDictionaryOfVariableBindings(scrubber);
//        NSArray *theConstraints = [NSLayoutConstraint constraintsWithVisualFormat:@"H:[scrubber(400)]" options:0 metrics:nil views:items];
//        [NSLayoutConstraint activateConstraints:theConstraints];

        // or you can do this:
        //[scrubber.widthAnchor constraintLessThanOrEqualToConstant:400].active = YES;
        
        scrubberItem.view = scrubber;
        
        return scrubberItem;
    }
    
    return nil;
}

#pragma mark -
#pragma mark ASCTabsControlDelegate

- (void)tabs:(ASCTabsControl *)control didSelectTab:(ASCTabView *)tab {
    NSUInteger index = [self indexOf:tab];

    if (index != NSNotFound) {
        [self updateTabInfoFor:tab];
        self.selectedIndex = index;
    } else {
        self.selectedIndex = -1;
    }
}

- (void)tabs:(ASCTabsControl *)control didUpdateTab:(ASCTabView *)tab {
    NSUInteger index = [self indexOf:tab];

    if (index != NSNotFound) {
        BOOL isChange = [self updateTabInfoFor:tab];

        if (self.tabsScrubber && isChange) {
            [self.tabsScrubber reloadItemsAtIndexes:[NSIndexSet indexSetWithIndex:index]];
        }
    }
}

- (void)tabs:(ASCTabsControl *)control didAddTab:(ASCTabView *)tab {
    NSUInteger index = [self indexOf:tab];

    if (index == NSNotFound) {
        ASCTabTouchBar *touchBarTab = [ASCTabTouchBar new];
        touchBarTab.type = ASCTabTouchBarTypePage;
        touchBarTab.uuid = tab.uuid;
        touchBarTab.title = tab.title;

        [self.tabs addObject:touchBarTab];
        [self invalidateTouchBar];

        __weak __typeof__(self) weakSelf = self;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.001 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            NSInteger lastIndex = weakSelf.tabs.count - 1;
            [[weakSelf.tabsScrubber animator] scrollItemAtIndex:lastIndex toAlignment:NSScrubberAlignmentNone];
        });
    }
}

- (void)tabs:(ASCTabsControl *)control didRemovedTab:(ASCTabView *)tab {
    NSUInteger index = [self indexOf:tab];

    if (index != NSNotFound) {
        [self.tabs removeObjectAtIndex:index];

        if (self.tabs.count < 1) {
            [self invalidateTouchBar];
        } else {
            __weak __typeof__(self) weakSelf = self;
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.001 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                [weakSelf.tabsScrubber removeItemsAtIndexes:[NSIndexSet indexSetWithIndex:index]];
            });
        }
    }
}

- (void)tabs:(ASCTabsControl *)control didReorderTab:(ASCTabView *)tab from:(NSInteger)oldIndex to:(NSInteger)newIndex {
    [self.tabs exchangeObjectAtIndex:oldIndex withObjectAtIndex:newIndex];

    //    [[self.tabsScrubber animator] moveItemAtIndex:oldIndex toIndex:newIndex]; // TODO: Don't call redraw for old index, check in new SDK
    
    __weak __typeof__(self) weakSelf = self;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [weakSelf.tabsScrubber reloadData];
        [weakSelf.tabsScrubber setSelectedIndex:newIndex];
        [weakSelf.tabsScrubber scrollItemAtIndex:newIndex toAlignment:NSScrubberAlignmentNone];
    });
}

@end

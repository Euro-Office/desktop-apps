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
//  ASCTabsControl.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class ASCTabsControl;
@class ASCTabView;

@interface ASCTabsMulticastDelegate : NSObject
- (void)addDelegate:(id)delegate;
@end

@protocol ASCTabsControlDelegate <NSObject>
@optional
- (void)tabs:(ASCTabsControl *)control didResize:(CGRect)rect;
- (void)tabs:(ASCTabsControl *)control didAddTab:(ASCTabView *)tab;
- (BOOL)tabs:(ASCTabsControl *)control willRemovedTab:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didRemovedTab:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didSelectTab:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didUpdateTab:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didReorderTab:(ASCTabView *)tab from:(NSInteger)oldIndex to:(NSInteger)newIndex;
- (BOOL)tabs:(ASCTabsControl *)control didDetachTab:(ASCTabView *)tab atScreenPoint:(NSPoint)screenPoint withEvent:(NSEvent *)event;
- (void)tabs:(ASCTabsControl *)control willShowContextMenuForTab:(ASCTabView *)tab withMenu:(NSMenu *)menu;
- (void)tabs:(ASCTabsControl *)control didRequestCloseTab:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didRequestCloseSavedTabs:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didRequestCloseAllTabs:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didRequestShowInFolderForTab:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didRequestDetachTab:(ASCTabView *)tab;
- (void)tabs:(ASCTabsControl *)control didRequestCreateNewForTab:(ASCTabView *)tab;
@end

@interface ASCTabsControl : NSControl
@property (nonatomic) NSMutableArray *tabs;
@property (nonatomic) CGFloat minTabWidth;
@property (nonatomic) CGFloat maxTabWidth;
@property (nonatomic, assign) id <ASCTabsControlDelegate> delegate;
@property (readonly) ASCTabsMulticastDelegate* multicastDelegate;
@property (nonatomic) BOOL tabPinAllowed;

- (void)addTab:(ASCTabView *)tab;
- (void)addTab:(ASCTabView *)tab selected:(BOOL)selected;
- (void)insertTab:(ASCTabView *)tab atIndex:(NSUInteger)index;
- (void)insertTab:(ASCTabView *)tab atIndex:(NSUInteger)index selected:(BOOL)selected;

- (void)removeTab:(ASCTabView *)tab animated:(BOOL)animated;
- (void)removeTab:(ASCTabView *)tab selected:(BOOL)selected animated:(BOOL)animated;
- (void)removeAllTabs;
- (void)selectTab:(ASCTabView *)tab;
- (void)updateTab:(ASCTabView *)tab;
- (void)selectNextTab;
- (void)selectPreviouseTab;

- (ASCTabView *)tabWithUUID:(NSString *)uuid;
- (ASCTabView *)selectedTab;
@end

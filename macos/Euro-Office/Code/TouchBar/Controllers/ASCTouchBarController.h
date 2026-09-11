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
//  ASCTouchBarController.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 02/01/2019.
//  Copyright (c) 2019 Ascensio System SIA. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ASCTabTouchBar.h"

NS_ASSUME_NONNULL_BEGIN

typedef void(^ASCTouchBarTapBlock)(id sender, NSString *senderId);

static NSString * const kCreationButtonIdentifier  = @"com.onlyoffice.touchbar.button.new.%@";
static NSString * const kStartPageButtonIdentifier = @"com.onlyoffice.touchbar.button.startpage";

@interface ASCTouchBarController : NSObject

@property (nonatomic) NSMutableArray<ASCTabTouchBar *> *tabs;
@property (nonatomic) NSInteger selectedIndex;
@property (nonatomic, copy) ASCTouchBarTapBlock onItemTap;

- (instancetype)init:(NSViewController *)viewController;
- (void)invalidateTouchBar;
- (NSTouchBar *)makeTouchBar;

@end

NS_ASSUME_NONNULL_END

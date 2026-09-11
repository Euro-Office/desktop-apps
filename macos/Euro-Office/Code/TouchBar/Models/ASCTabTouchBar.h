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
//  ASCTabTouchBar.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 02/01/2019.
//  Copyright (c) 2019 Ascensio System SIA. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, ASCTabTouchBarType)
{
    ASCTabTouchBarTypeRoot          = 0,
    ASCTabTouchBarTypeDocument      = 1,
    ASCTabTouchBarTypeSpreadsheet   = 2,
    ASCTabTouchBarTypePresentation  = 3,
    ASCTabTouchBarTypePortal        = 4,
    ASCTabTouchBarTypePage          = 5,
    ASCTabTouchBarTypePdf           = 6
};

NS_ASSUME_NONNULL_BEGIN

@interface ASCTabTouchBar : NSObject

@property (nonatomic) ASCTabTouchBarType type;
@property (nonatomic) NSString * uuid;
@property (nonatomic) NSString * title;

@end

NS_ASSUME_NONNULL_END

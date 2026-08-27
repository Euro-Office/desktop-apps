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
//  ASCExternalController.h
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 18/09/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol ASCExternalDelegate <NSObject>
@optional
- (NSString *)onApplicationName;
- (NSString *)onApplicationNameShort;
- (NSString *)onAppInfo:(NSString *)key;
- (NSString *)onAppPreferredLanguage;
- (NSString *)onCommercialInfo;
- (BOOL)onShouldOpenFile:(NSString *)filePath;
- (void)onMainWindowLoaded:(id)controller;
- (void)onCommonViewDidLoad:(id)controller;
- (void)onCefMenuEvent:(void *)event;
@end

@interface ASCExternalController : NSObject
@property (nonatomic, assign) id <ASCExternalDelegate> delegate;
+ (instancetype)shared;
@end

NS_ASSUME_NONNULL_END

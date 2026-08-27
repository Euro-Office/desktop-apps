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
//  ASCTabCloseButtonCell.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/10/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTabCloseButtonCell.h"
#import "ASCTabViewCell.h"

@interface ASCTabCloseButtonCell ()

@end

@implementation ASCTabCloseButtonCell

- (id)initTextCell:(NSString *)string {
    self = [super initTextCell:string];
    if (self) {
        self.image = [self isParentLight]
            ? [NSImage imageNamed:@"tab_close_dark_normal"]
            : [NSImage imageNamed:@"tab_close_light_normal"];
    }
    
    return self;
}

- (void)mouseEntered:(NSEvent *)theEvent {
    [super mouseEntered:theEvent];

    self.image = [self isParentLight]
        ? [NSImage imageNamed:@"tab_close_dark_hover"]
        : [NSImage imageNamed:@"tab_close_light_hover"];
}

- (void)mouseExited:(NSEvent *)theEvent {
    [super mouseExited:theEvent];

    self.image = [self isParentLight]
        ? [NSImage imageNamed:@"tab_close_dark_normal"]
        : [NSImage imageNamed:@"tab_close_light_normal"];
}

- (void)mouseDown:(NSEvent *)theEvent {
    [super mouseDown:theEvent];

    self.image = [self isParentLight]
        ? [NSImage imageNamed:@"tab_close_dark_active"]
        : [NSImage imageNamed:@"tab_close_light_active"];
}

- (void)mouseUp:(NSEvent *)theEvent {
    [super mouseUp:theEvent];

    self.image = [self isParentLight]
        ? [NSImage imageNamed:@"tab_close_dark_normal"]
        : [NSImage imageNamed:@"tab_close_light_normal"];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
    if (self.image && self.imagePosition != NSNoImage) {
        [self drawImage:self.image withFrame:cellFrame inView:controlView];
    }
}

- (BOOL)isParentActive {
    id target = [self target];

    if (target && [target isKindOfClass:[NSView class]]) {
        NSButtonCell * buttonCell = [target cell];

        if (buttonCell) {
            return (BOOL)[buttonCell state];
        }
    }
    return false;
}

- (BOOL)isParentLight {
    id target = [self target];

    if (target && [target isKindOfClass:[NSView class]]) {
        ASCTabViewCell * buttonCell = [target cell];

        if (buttonCell) {
            return buttonCell.isLight;
        }
    }

    return NO;
}

@end

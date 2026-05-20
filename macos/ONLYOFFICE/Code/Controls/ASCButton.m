/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

//
//  ASCButton.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/10/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCButton.h"
#import "ASCButtonCell.h"

@implementation ASCButton

- (id)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];

    if (self) {
        [self _initialize];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    
    if (self) {
        [self _initialize];
    }
    
    return self;
}

- (void)_initialize {
    NSTrackingAreaOptions focusTrackingAreaOptions = NSTrackingActiveInActiveApp;
    focusTrackingAreaOptions |= NSTrackingMouseEnteredAndExited;
    focusTrackingAreaOptions |= NSTrackingAssumeInside;
    focusTrackingAreaOptions |= NSTrackingInVisibleRect;
    focusTrackingAreaOptions |= NSTrackingMouseMoved;
    
    NSTrackingArea *focusTrackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                                     options:focusTrackingAreaOptions
                                                                       owner:self
                                                                    userInfo:nil];
    [self addTrackingArea:focusTrackingArea];
}

- (void)mouseEntered:(NSEvent *)theEvent {
    if ([self.cell isKindOfClass:[ASCButtonCell class]]) {
        [self.cell mouseEntered:theEvent];
    }
         
    [self setNeedsDisplay];
}

- (void)mouseExited:(NSEvent *)theEvent {
    [super mouseExited:theEvent];
    
    if ([self.cell isKindOfClass:[ASCButtonCell class]]) {
        [self.cell mouseExited:theEvent];
    }
    
    [self setNeedsDisplay];
}

- (void)mouseMoved:(NSEvent *)theEvent {
    [super mouseMoved:theEvent];
    
    if ([self.cell isKindOfClass:[ASCButtonCell class]]) {
        [self.cell mouseMoved:theEvent];
    }
    
    [self setNeedsDisplay];
}

- (void)mouseDown:(NSEvent *)theEvent {
    if (theEvent.clickCount < 2) {
        [super mouseDown:theEvent];
        
        if ([self.cell isKindOfClass:[ASCButtonCell class]]) {
            [self.cell mouseDown:theEvent];
        }
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    [super mouseUp:theEvent];
    
    if (self.cell && [self.cell isKindOfClass:[ASCButtonCell class]]) {
        [self.cell mouseUp:theEvent];
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
}

@end

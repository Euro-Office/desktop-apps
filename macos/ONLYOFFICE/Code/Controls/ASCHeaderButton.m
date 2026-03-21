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
//  ASCHeaderButton.m
//  ONLYOFFICE
//
//  Copyright (c) 2026 Ascensio System SIA. All rights reserved.
//

#import "ASCHeaderButton.h"

@interface ASCHeaderButton()
@property (nonatomic) NSTrackingArea *hoverTrackingArea;
@property (nonatomic) BOOL isHovered;
@end

@implementation ASCHeaderButton

- (instancetype)init {
    self = [super init];
    if (self) {
        self.bgHoverColor = [NSColor colorWithWhite:0.0 alpha:0.15];
    }
    return self;
}

- (void)updateTrackingAreas {
    [super updateTrackingAreas];

    if (self.hoverTrackingArea) {
        [self removeTrackingArea:self.hoverTrackingArea];
    }

    self.hoverTrackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                          options:NSTrackingMouseEnteredAndExited |
                                                                  NSTrackingActiveInKeyWindow |
                                                                  NSTrackingInVisibleRect
                                                            owner:self
                                                         userInfo:nil];

    [self addTrackingArea:self.hoverTrackingArea];
}

- (void)viewDidMoveToWindow {
    [super viewDidMoveToWindow];
    
    if (self.window) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidResignKey:)
                                                     name:NSWindowDidResignKeyNotification
                                                   object:self.window];
    }
}

- (void)windowDidResignKey:(NSNotification *)note {
    if (_isHovered) {
        _isHovered = NO;
        [self setNeedsDisplay:YES];
    }
}

- (void)mouseEntered:(NSEvent *)event {
    [super mouseEntered:event];
    _isHovered = YES;
    [self setNeedsDisplay:YES];
}

- (void)mouseExited:(NSEvent *)event {
    [super mouseExited:event];
    _isHovered = NO;
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect {
    if (_isHovered) {
        [_bgHoverColor set];
        NSRectFillUsingOperation(self.bounds, NSCompositingOperationSourceOver);
    }
    [super drawRect:dirtyRect];
}

@end

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
//  ASCMenuButtonCell.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 12/17/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCMenuButtonCell.h"
#import "NSColor+Extensions.h"

@interface ASCMenuButtonCell()
@end

@implementation ASCMenuButtonCell

- (id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    
    if (self) {
        [self setLineBreakMode:NSLineBreakByTruncatingTail];
        
        self.bgColor            = kColorRGB(96, 101, 106);
        self.bgHoverColor       = kColorRGB(105, 110, 116);
        self.bgActiveColor      = kColorRGB(255, 255, 255);
        self.textColor          = kColorRGB(255, 255, 255);
        self.textActiveColor    = kColorRGB(102, 102, 102);
        self.lineColor          = kColorRGB(79, 84, 88);
    }
    
    return self;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
//        [super drawWithFrame:cellFrame inView:controlView];

    CGFloat rectangleCornerRadius = 0;
    
    // Color Declarations
    NSColor * color;
    
    if (self.state) {
        color = (self.isHover) ? self.bgActiveColor : self.bgActiveColor;
    } else {
        color = (self.isHover) ? self.bgHoverColor : self.bgColor;
    }
    
    //// Rectangle Drawing
    NSRect rectangleRect = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y, cellFrame.size.width - 1, cellFrame.size.height);
    NSRect rectangleInnerRect = NSInsetRect(rectangleRect, rectangleCornerRadius, rectangleCornerRadius);
    NSBezierPath* rectanglePath = [NSBezierPath bezierPath];
    [rectanglePath appendBezierPathWithArcWithCenter: NSMakePoint(NSMinX(rectangleInnerRect), NSMinY(rectangleInnerRect))
                                              radius: rectangleCornerRadius
                                          startAngle: 180
                                            endAngle: 270];
    [rectanglePath appendBezierPathWithArcWithCenter: NSMakePoint(NSMaxX(rectangleInnerRect), NSMinY(rectangleInnerRect))
                                              radius: rectangleCornerRadius
                                          startAngle: 270
                                            endAngle: 360];
    [rectanglePath lineToPoint: NSMakePoint(NSMaxX(rectangleRect), NSMaxY(rectangleRect))];
    [rectanglePath lineToPoint: NSMakePoint(NSMinX(rectangleRect), NSMaxY(rectangleRect))];
    [rectanglePath closePath];
    [color setFill];
    [rectanglePath fill];
    
    if (!self.state) {
        //// Bottom Line Drawing
        NSBezierPath* bottomRectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(rectangleRect), NSHeight(rectangleRect) - 1, NSWidth(rectangleRect), 1)];
        [self.lineColor setFill];
        [bottomRectanglePath fill];
    }
    
    if (self.title) {
        [self drawTitle:[self attributedTitle] withFrame:cellFrame inView:controlView];
    }

    if (self.image) {
        [self drawImage:self.image withFrame:cellFrame inView:controlView];
    }
}

- (NSCellStyleMask)highlightsBy {
    return NSNoCellMask;
}

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView {
    return [super drawTitle:title withFrame:CGRectMake(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height) inView:controlView];
}

- (NSAttributedString *)attributedTitle {
    NSMutableAttributedString *attributedTitle  = [[super attributedTitle] mutableCopy];
    NSMutableParagraphStyle *paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
//    NSFont *font = [NSFont boldSystemFontOfSize:11];
    NSColor *color = self.textColor;
    
    if (self.state) {
        color = self.textActiveColor;
    }
    
    [paragraphStyle setAlignment:NSCenterTextAlignment];
    [paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [attributedTitle addAttributes:@{
                                     NSForegroundColorAttributeName:color,
                                     NSParagraphStyleAttributeName:paragraphStyle
//                                     NSFontAttributeName:font
                                     }
                             range:NSMakeRange(0, attributedTitle.length)];
    return attributedTitle;
}
@end

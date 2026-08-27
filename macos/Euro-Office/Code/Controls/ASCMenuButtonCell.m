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

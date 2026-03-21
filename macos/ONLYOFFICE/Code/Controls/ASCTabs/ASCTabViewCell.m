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
//  ASCTabViewCell.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/10/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "ASCTabViewCell.h"
#import "NSColor+Extensions.h"
#import "NSImage+Extensions.h"
#import "ASCButtonCell.h"
#import "NSApplication+Extensions.h"
#import "ASCThemesController.h"

@interface ASCTabViewCell()
@property (nonatomic) NSImageView * animatedImageView;
@property (nonatomic, weak) NSView * parentView;
@property (nonatomic, readonly) CALayer * loaderLayer;
@property (nonatomic, readonly) CABasicAnimation * loaderAnimation;
@end

@implementation ASCTabViewCell

@synthesize loaderLayer = _loaderLayer;
@synthesize loaderAnimation = _loaderAnimation;
@synthesize isProcessing = _isProcessing;
@synthesize isLight = _isLight;

#pragma mark - Properties

- (CALayer *)loaderLayer {
    if (_loaderLayer) {
        return _loaderLayer;
    }

    NSImage * loaderImage = [ASCThemesController isDarkWindowAppearance]
        ? [NSImage imageNamed:@"tab-loader-light"]
        : [NSImage imageNamed:@"tab-loader-dark"];

    if (nil == loaderImage || nil == self.parentView) {
        return nil;
    }

    CGFloat desiredScaleFactor = [[NSApp mainWindow] backingScaleFactor];
    CGFloat actualScaleFactor = [loaderImage recommendedLayerContentsScale:desiredScaleFactor];
    id layerContents = [loaderImage layerContentsForContentsScale:actualScaleFactor];
    NSSize size = [loaderImage size];
    CGRect rect = CGRectMake(
                             8,
                             (CGRectGetHeight(self.parentView.bounds) - size.width) * .5 - 1,
                             size.width,
                             size.height);


    CALayer * layer = [CALayer new];
//    layer.backgroundColor = NSColor.redColor.CGColor;
    layer.bounds = rect;
    layer.anchorPoint = CGPointMake(0.5, 0.5);
    layer.position = CGPointMake(CGRectGetMidX(rect), CGRectGetMidY(rect));
    layer.contents = layerContents;
    layer.contentsScale = actualScaleFactor;
    layer.contentsGravity = kCAGravityCenter;
    layer.zPosition = 1000;
    layer.hidden = !_isProcessing;

    _loaderLayer = layer;
    return _loaderLayer;
}

- (CABasicAnimation *)loaderAnimation {
    if (_loaderAnimation) {
        return _loaderAnimation;
    }

    CABasicAnimation * animation = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];

    animation.fromValue = 0;
    animation.toValue = [NSNumber numberWithFloat: M_PI * 4];
    animation.duration = 2;
    animation.repeatCount = HUGE_VALF;
    animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
    animation.removedOnCompletion = true;

    _loaderAnimation = animation;

    return animation;
}

- (void)setIsProcessing:(BOOL)isProcessing {
    if (_isProcessing != isProcessing) {
        _isProcessing = isProcessing;
        _isProcessing ? [self startProcessing] : [self stopProcessing];
    }
}

- (void)setIsLight:(BOOL)isLight {
    if (_isLight != isLight) {
        _isLight = isLight;

        NSImage * loaderImage = isLight
            ? [NSImage imageNamed:@"tab-loader-dark"]
            : [NSImage imageNamed:@"tab-loader-light"];

        CGFloat desiredScaleFactor = [[NSApp mainWindow] backingScaleFactor];
        CGFloat actualScaleFactor = [loaderImage recommendedLayerContentsScale:desiredScaleFactor];

        id layerContents = [loaderImage layerContentsForContentsScale:actualScaleFactor];

        [self.loaderLayer setContents:layerContents];
        [self.loaderLayer setContentsScale:actualScaleFactor];

        [self updateCloseCell];
    }
}

#pragma mark - Lifecycle Methods

- (id)initTextCell:(NSString *)string {
    self = [super initTextCell:string];
    if (self) {
        [self setLineBreakMode:NSLineBreakByTruncatingTail];


        if (@available(macOS 10.13, *)) {
            self.inactiveColor          = [NSColor colorNamed:@"tab-inactiveColor"];
            self.activeColor            = [NSColor colorNamed:@"tab-activeColor"];
            self.hoverInactiveColor     = [NSColor colorNamed:@"tab-hoverInactiveColor"];
            self.hoverActiveColor       = [NSColor colorNamed:@"tab-hoverActiveColor"];
            self.clickColor             = [NSColor colorNamed:@"tab-clickColor"];
            self.activeTextColor        = [NSColor colorNamed:@"tab-activeTextColor"];
            self.inactiveTextColor      = [NSColor colorNamed:@"tab-inactiveTextColor"];
            self.inactiveBorderColor    = [NSColor colorNamed:@"tab-inactiveBorderColor"];
        } else {
            self.inactiveColor          = UIColorFromRGBA(0xe5e5e5, 0);
            self.activeColor            = UIColorFromRGBA(0x000000, 0.1);
            self.hoverInactiveColor     = UIColorFromRGBA(0x000000, 0.1);
            self.hoverActiveColor       = UIColorFromRGB(0xe5e5e5);
            self.clickColor             = UIColorFromRGB(0xe5e5e5);
            self.activeTextColor        = UIColorFromRGB(0x000000);
            self.inactiveTextColor      = UIColorFromRGB(0x000000);
            self.inactiveBorderColor    = UIColorFromRGBA(0x000000, 0);
        }

        _parentView = nil;
        _loaderLayer = nil;
        _loaderAnimation = nil;
    }

    if (_isProcessing) {
        [self startProcessing];
    }

    return self;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
//    [super drawWithFrame:cellFrame inView:controlView];

    self.parentView = controlView;

    CGFloat rectangleCornerRadius = 0;
    
    // Color Declarations
    NSColor * color;
    
    if (self.state) {
        color = (self.isHover) ? self.activeColor : self.activeColor;
    } else {
        color = (self.isHover) ? self.hoverInactiveColor : self.inactiveColor;
//        (self.isHover) ? NSLog(@"Hover %@ TRUE", [self className]) : NSLog(@"Hover %@ FALSE", [self className]);
    }

    self.isLight = [color isLight] || ([ASCThemesController isDarkWindowAppearance] ? false : [color alphaComponent] < 0.5);

    // Rectangle Drawing
    NSRect rectangleRect = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y, cellFrame.size.width, cellFrame.size.height);
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
        // Bottom Line Drawing
        NSBezierPath* bottomRectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(rectangleRect), NSHeight(rectangleRect), NSWidth(rectangleRect), 1)];
        [self.inactiveBorderColor setFill];
        [bottomRectanglePath fill];
    }

    if (true/*!self.state*/) {
        NSBezierPath * leftRectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(-1, 0, 1, NSHeight(rectangleRect))];
        NSBezierPath * rightRectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMaxX(rectangleRect) - 1, 0, 1, NSHeight(rectangleRect))];
        NSColor * lineColor = kColorRGBA(0, 0, 0, 0.2);
        [lineColor setFill];
        [leftRectanglePath fill];
        [rightRectanglePath fill];
    }

    if (self.title) {
        [self drawTitle:[self attributedTitle] withFrame:cellFrame inView:controlView];
    }

    if (self.parentView && self.loaderLayer && self.loaderLayer.superlayer != self.parentView.layer) {
        [self.parentView.layer addSublayer:self.loaderLayer];
    }

    if (_isProcessing) {
        if ([self.loaderLayer animationForKey:@"rotateAnimation"] == nil) {
            [self startProcessing];
        }
    }

    if (self.image && self.imagePosition != NSNoImage) {
        if (!_isProcessing) {
            [self drawImage:self.image withFrame:cellFrame inView:controlView];
            [self stopProcessing];
        }
    }
}

- (void)startProcessing {
    self.loaderLayer.hidden = false;

    if ([self.loaderLayer animationForKey:@"rotateAnimation"]) {
        [self.loaderLayer removeAnimationForKey:@"rotateAnimation"];
        [self.loaderLayer removeFromSuperlayer];
    }

    if (self.parentView && self.loaderLayer && self.loaderLayer.superlayer != self.parentView.layer) {
        [self.parentView.layer addSublayer:self.loaderLayer];
    }

    [CATransaction begin];
    [self.loaderLayer addAnimation:self.loaderAnimation forKey:@"rotateAnimation"];
    [CATransaction commit];
}

- (void)stopProcessing {
    self.loaderLayer.hidden = true;
    
    if ([self.loaderLayer animationForKey:@"rotateAnimation"]) {
        [self.loaderLayer removeAnimationForKey:@"rotateAnimation"];
        [self.loaderLayer removeFromSuperlayer];
    }
}

- (NSCellStyleMask)highlightsBy {
    return NSNoCellMask;
}

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView {
    NSSize size = [image size];
    CGRect rect = CGRectMake(8, (CGRectGetHeight(frame) - size.width) * .5 - 1, size.width, size.height);
    if ( [self userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft )
        rect.origin.x = frame.size.width - 8 - size.width;

    [super drawImage:image withFrame:rect inView:controlView];
}

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView {
    CGFloat leftOffset  = 0.f;
    CGFloat rightOffset = 0.f;
    
    if (self.image) {
        leftOffset = 8 + self.image.size.width + 5;
    }
    
    if (self.closeButton) {
        rightOffset = CGRectGetHeight(self.closeButton.frame) * 1.5 + 2;
    }
    
    if ( [self userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft ) {
        CGFloat t = leftOffset;
        leftOffset = rightOffset;
        rightOffset = t;
    }

    if (CGRectGetWidth(frame) - leftOffset - rightOffset > 15) {
        return [super drawTitle:title withFrame:CGRectMake(frame.origin.x + leftOffset, frame.origin.y, frame.size.width - rightOffset - leftOffset, frame.size.height) inView:controlView];
    } else {
        return [super drawTitle:[[NSAttributedString alloc] initWithString:@""] withFrame:CGRectZero inView:controlView];
    }
    
}

- (NSAttributedString *)attributedTitle {
    NSMutableAttributedString *attributedTitle  = [[super attributedTitle] mutableCopy];
    NSMutableParagraphStyle *paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    NSFont *font = [NSFont systemFontOfSize:11];
    NSColor *color = self.inactiveTextColor;

    if (self.state) {
        color = self.activeTextColor;
    }
    
    if ( [self userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft )
        [paragraphStyle setAlignment:NSRightTextAlignment];
    else [paragraphStyle setAlignment:NSLeftTextAlignment];
    [paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
    
    [attributedTitle addAttributes:@{
                                     NSForegroundColorAttributeName:color,
                                     NSParagraphStyleAttributeName:paragraphStyle,
                                     NSFontAttributeName:font
                                     }
                             range:NSMakeRange(0, attributedTitle.length)];
    return attributedTitle;
}
//
//- (void)mouseEntered:(NSEvent *)theEvent {
//    [super mouseEntered:theEvent];
//}
//
//- (void)mouseExited:(NSEvent *)theEvent {
//    [super mouseExited:theEvent];
//}
//
//- (void)mouseMoved:(NSEvent *)theEvent {
//    [super mouseMoved:theEvent];
//}
//
//- (void)mouseDown:(NSEvent *)theEvent {
//    [super mouseDown:theEvent];
//}
//
//- (void)mouseUp:(NSEvent *)theEvent {
//    [super mouseUp:theEvent];
//}

- (void)updateCloseCell {
    NSEvent * event = [NSApp currentEvent];

    if (self.parentView) {
        [self.parentView setNeedsDisplay:YES];
    }

    ASCButtonCell * closeCell = [[self closeButton] cell];
    if (closeCell) {
        [closeCell mouseExited:event];
    }
}

@end

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
//  NSView+Extensions.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 11/04/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import <objc/runtime.h>
#import <QuartzCore/QuartzCore.h>
#import "NSView+Extensions.h"

static NSString * const kASCUuidPropertyKey = @"ascUuidPropertyKey";

@implementation NSView (Extensions)

#pragma mark - Properties

- (NSColor *)backgroundColor {
    if (self.layer && self.layer.backgroundColor) {
        [NSColor colorWithCGColor: self.layer.backgroundColor];
    }
    return nil;
}

- (void)setBackgroundColor:(NSColor *)backgroundColor {
    self.wantsLayer = YES;
    self.layer.backgroundColor = backgroundColor.CGColor;
}

- (NSColor *)borderColor {
    if (self.layer && self.layer.borderColor) {
        [NSColor colorWithCGColor: self.layer.borderColor];
    }
    return nil;
}

- (void)setBorderColor:(NSColor *)borderColor {
    self.wantsLayer = YES;
    self.layer.borderColor = borderColor.CGColor;
}

- (NSColor *)shadowColor {
    if (self.layer && self.layer.shadowColor) {
        [NSColor colorWithCGColor: self.layer.shadowColor];
    }
    return nil;
}

- (void)setShadowColor:(NSColor *)shadowColor {
    self.wantsLayer = YES;
    self.layer.shadowColor = shadowColor.CGColor;
}

- (CGFloat)borderWidth {
    if (self.layer) {
        return self.layer.borderWidth;
    }
    return 0.0;
}

- (void)setBorderWidth:(CGFloat)borderWidth {
    self.wantsLayer = YES;
    self.layer.borderWidth = borderWidth;
}

- (CGFloat)cornerRadius {
    if (self.layer) {
        return self.layer.cornerRadius;
    }
    return 0.0;
}

- (void)setCornerRadius:(CGFloat)cornerRadius {
    self.wantsLayer = YES;
    self.layer.cornerRadius = cornerRadius;
}

- (CGFloat)width {
    return self.frame.size.width;
}

- (void)setWidth:(CGFloat)width {
    NSRect newFrame = self.frame;
    newFrame.size.width = width;
    newFrame.size.height = self.frame.size.height;
    self.frame = newFrame;
}

- (CGFloat)height {
    return self.frame.size.height;
}

- (void)setHeight:(CGFloat)height {
    NSRect newFrame = self.frame;
    newFrame.size.width = self.frame.size.width;
    newFrame.size.height = height;
    self.frame = newFrame;
}

- (CGSize)size {
    return self.frame.size;
}

- (void)setSize:(CGSize)size {
    self.width = size.width;
    self.height = size.height;
}

- (CGSize)shadowOffset {
    if (self.layer) {
        return self.layer.shadowOffset;
    }
    return CGSizeZero;
}

- (void)setShadowOffset:(CGSize)shadowOffset {
    self.wantsLayer = YES;
    self.layer.shadowOffset = shadowOffset;
}

- (CGFloat)shadowOpacity {
    if (self.layer) {
        return self.layer.shadowOpacity;
    }
    return 0.0;
}

- (void)setShadowOpacity:(CGFloat)shadowOpacity {
    self.wantsLayer = YES;
    self.layer.shadowOpacity = shadowOpacity;
}

- (CGFloat)shadowRadius {
    if (self.layer) {
        return self.layer.shadowRadius;
    }
    return 0.0;
}

- (void)setShadowRadius:(CGFloat)shadowRadius {
    self.wantsLayer = YES;
    self.layer.shadowRadius = shadowRadius;
}

- (NSInteger)uuidTag {
    NSNumber * uuidTagNumber = objc_getAssociatedObject(self, (__bridge const void *)(kASCUuidPropertyKey));
    if (uuidTagNumber) {
        return [uuidTagNumber integerValue];
    }
    return -1;
}

- (void)setUuidTag:(NSInteger)uuidTag {
    NSNumber * uuidNumber = [NSNumber numberWithInteger:uuidTag];
    objc_setAssociatedObject(self, (__bridge const void *)(kASCUuidPropertyKey), uuidNumber, OBJC_ASSOCIATION_RETAIN);
}

#pragma mark - Methods

- (void)removeAllConstraints {
    NSView *superview = self.superview;
    while (superview != nil) {
        for (NSLayoutConstraint *constraint in superview.constraints) {
            if (constraint.firstItem == self || constraint.secondItem == self) {
                [superview removeConstraint:constraint];
            }
        }
        superview = superview.superview;
    }

    [self removeConstraints:self.constraints];
    self.translatesAutoresizingMaskIntoConstraints = YES;
}

- (instancetype)duplicate {
    NSData * archivedView = [NSKeyedArchiver archivedDataWithRootObject:self];

    if (archivedView) {
        return [NSKeyedUnarchiver unarchiveObjectWithData:archivedView];
    }

    return nil;
}

- (CAKeyframeAnimation *)shakeAnimation:(NSRect)frame {
    static int numberOfShakes = 3;
    static float durationOfShake = 0.5f;
    static float vigourOfShake = 0.02f;

    CAKeyframeAnimation *shakeAnimation = [CAKeyframeAnimation animation];

    CGMutablePathRef shakePath = CGPathCreateMutable();
    CGPathMoveToPoint(shakePath, NULL, NSMinX(frame), NSMinY(frame));
    int index;

    for (index = 0; index < numberOfShakes; ++index) {
        CGPathAddLineToPoint(shakePath, NULL, NSMinX(frame) - frame.size.width * vigourOfShake, NSMinY(frame));
        CGPathAddLineToPoint(shakePath, NULL, NSMinX(frame) + frame.size.width * vigourOfShake, NSMinY(frame));
    }
    CGPathCloseSubpath(shakePath);
    shakeAnimation.path = shakePath;
    shakeAnimation.duration = durationOfShake;
    return shakeAnimation;
}

- (void)shake {
    self.wantsLayer = YES;

    CAKeyframeAnimation * animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.translation.x"];
    animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
    animation.removedOnCompletion = true;
    animation.duration = 0.6;
    animation.values = @[@(-20.0), @(20.0), @(-20.0), @(20.0), @(-10.0), @(10.0), @(-5.0), @(5.0), @(0.0) ];
    [self.layer addAnimation:animation forKey:@"shake"];
}

- (NSImage *)imageRepresentation {
    BOOL wasHidden = self.isHidden;
    CGFloat wantedLayer = self.wantsLayer;

    self.hidden = NO;
    self.wantsLayer = YES;

    NSImage *image = [[NSImage alloc] initWithSize:self.bounds.size];
    [image lockFocus];
    CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
    [self.layer renderInContext:ctx];
    [image unlockFocus];

    self.wantsLayer = wantedLayer;
    self.hidden = wasHidden;

    return image;
}

- (nullable NSImage *)windowScreenshot {
    if (self.window) {
        NSRect viewFrameInWindow = [self convertRect:self.bounds toView:nil];
        NSRect windowFrame = [self.window convertRectToScreen:viewFrameInWindow];
        CGRect mainDisplayBounds = CGDisplayBounds(CGMainDisplayID());
        
        CGFloat cgX = windowFrame.origin.x;
        CGFloat cgY = mainDisplayBounds.size.height - windowFrame.origin.y - windowFrame.size.height;
        
        CGImageRef cgImage = CGWindowListCreateImage
        (
         CGRectMake(cgX, cgY, windowFrame.size.width, windowFrame.size.height),
         kCGWindowListOptionIncludingWindow,
         (CGWindowID)[self.window windowNumber],
         kCGWindowImageBoundsIgnoreFraming | kCGWindowImageShouldBeOpaque
         );
        
        NSImage *snapshot = nil;
        if (cgImage) {
            snapshot = [[NSImage alloc] initWithCGImage:cgImage size:viewFrameInWindow.size];
            CGImageRelease(cgImage);
        }
        
        return snapshot;
    }
    
    return nil;
}

- (nullable NSView *)subviewOfClassName:(NSString * _Nonnull)className {
    Class cls = NSClassFromString(className);
    
    if (!cls) {
        return nil;
    }

    for (NSView *subview in self.subviews) {
        if ([subview isKindOfClass:cls]) {
            return subview;
        }
        
        NSView *found = [subview subviewOfClassName:className];
        
        if (found) {
            return found;
        }
    }
    return nil;
}

@end

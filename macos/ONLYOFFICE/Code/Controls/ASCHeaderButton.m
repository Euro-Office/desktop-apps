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
#import <QuartzCore/QuartzCore.h>

@interface ASCHeaderButton()
@property (nonatomic) NSTrackingArea *hoverTrackingArea;
@property (nonatomic) BOOL isHovered;
@property (nonatomic, strong) NSImage * spinBaseImage;
@property (nonatomic, strong) NSTimer * spinTimer;
@property (nonatomic, assign) CFTimeInterval spinStartTime;
@property (nonatomic, assign) BOOL isSettingSpinFrame;
@end

@implementation ASCHeaderButton

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
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
        [self p_applyHoverState];
    }
}

- (void)mouseEntered:(NSEvent *)event {
    _isHovered = YES;
    [self p_applyHoverState];
}

- (void)mouseExited:(NSEvent *)event {
    _isHovered = NO;
    [self p_applyHoverState];
}

- (void)p_applyHoverState {
    [self p_updateTitleColor];
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect {
    if (_isHovered && _bgHoverColor) {
        [_bgHoverColor set];
        NSRectFillUsingOperation(self.bounds, NSCompositingOperationSourceOver);
    }
    [super drawRect:dirtyRect];
}

- (void)p_updateTitleColor {
    if (self.title.length == 0) return;

    NSColor *color = _isHovered ? _textHoverColor : _textColor;
    if (!color) return;

    NSMutableAttributedString *attr = [[NSMutableAttributedString alloc] initWithAttributedString:self.attributedTitle];
    NSRange all = NSMakeRange(0, attr.length);
    [attr addAttribute:NSForegroundColorAttributeName value:color range:all];
    if (self.font) {
        [attr addAttribute:NSFontAttributeName value:self.font range:all];
    }
    self.attributedTitle = attr;
}

- (void)setTextColor:(NSColor *)textColor {
    _textColor = textColor;
    [self p_updateTitleColor];
}

- (void)setTextHoverColor:(NSColor *)textHoverColor {
    _textHoverColor = textHoverColor;
    [self p_updateTitleColor];
}

- (void)setTitle:(NSString *)title {
    [super setTitle:title];
    [self p_updateTitleColor];
}

- (void)setFont:(NSFont *)font {
    [super setFont:font];
    [self p_updateTitleColor];
}

- (void)setImage:(NSImage *)image {
    if (!_isSettingSpinFrame) {
        _spinBaseImage = image;
    }
    [super setImage:image];
}

- (void)p_tickSpinAnimation {
    NSImage *base = self.spinBaseImage;
    if (!base) return;

    const CGFloat w = base.size.width;
    const CGFloat h = base.size.height;
    const CFTimeInterval period = 1.5; // seconds per full clockwise revolution

    CFTimeInterval elapsed = CACurrentMediaTime() - self.spinStartTime;
    CGFloat angle = -fmod(elapsed / period, 1.0) * 360.0;

    NSImage *frame = [[NSImage alloc] initWithSize:NSMakeSize(w, h)];
    [frame lockFocus];
    NSAffineTransform *t = [NSAffineTransform transform];
    [t translateXBy:w / 2.0 yBy:h / 2.0];
    [t rotateByDegrees:angle];
    [t translateXBy:-w / 2.0 yBy:-h / 2.0];
    [t concat];
    [base drawInRect:NSMakeRect(0, 0, w, h)
            fromRect:NSZeroRect
           operation:NSCompositingOperationSourceOver
            fraction:1.0];
    [frame unlockFocus];

    _isSettingSpinFrame = YES;
    [self setImage:frame];
    _isSettingSpinFrame = NO;
}

- (BOOL)isSpinning {
    return self.spinTimer != nil;
}

- (void)startAnimation {
    if (self.spinTimer) return; // already running
    self.spinBaseImage = self.image;
    self.spinStartTime = CACurrentMediaTime();
    self.spinTimer = [NSTimer timerWithTimeInterval:1.0 / 60.0
                                            target:self
                                          selector:@selector(p_tickSpinAnimation)
                                          userInfo:nil
                                           repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:self.spinTimer forMode:NSRunLoopCommonModes];
}

- (void)stopAnimation {
    if (self.spinTimer) {
        [self.spinTimer invalidate];
        self.spinTimer = nil;
    }
    self.spinBaseImage = nil;
}

@end

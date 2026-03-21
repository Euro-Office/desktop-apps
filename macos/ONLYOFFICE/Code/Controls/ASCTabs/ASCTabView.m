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
//  ASCTabView.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/7/15.
//  Copyright (c) 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCTabView.h"
#import "ASCTabCloseButtonCell.h"
#import "ASCTabViewCell.h"
#import "NSColor+Extensions.h"
#import "NSApplication+Extensions.h"
#import "ASCThemesController.h"

static NSUInteger const kASTabViewCloseButtonSize = 12;

@interface ASCTabView()
@property (nonatomic) NSButton * close;
@property (nonatomic) NSMutableArray * icons;
@end

@implementation ASCTabView

@synthesize isProcessing = _isProcessing;

- (id)init {
    self = [super init];
    
    if (self) {
        [self initialize];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    
    if (self) {
        [self initialize];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    
    if (self) {
        [self initialize];
    }
    
    return self;
}

- (instancetype)copyWithZone:(NSZone *)zone
{
//    ASCTabView * copy = [[ASCTabView allocWithZone:zone] initWithFrame:self.frame];
    ASCTabView * copy = [[ASCTabView allocWithZone:zone] initWithFrame:CGRectZero];
    copy.type = self.type;

    ASCTabViewCell * cellCopy = [self.cell copy];
    [copy setCell:cellCopy];
    [copy setState:[self state]];
    [copy setNeedsDisplay];

    copy.frame = self.frame;
    return copy;
}

- (void)refreshPortalTabIconset {
    NSString * normalIcon = [ASCThemesController isDarkWindowAppearance] ? @"icon_tab_portal_inactive" : @"icon_tab_portal_active",
            * activeIcon = [ASCThemesController isCurrentThemeDark] ? @"icon_tab_portal_inactive" : @"icon_tab_portal_active";

    _icons[ASCTabViewTypePortal] = @{@"normal": normalIcon, @"active": activeIcon};
}

- (void)initialize {   
    _uuid = [[NSUUID UUID] UUIDString];
    
    _icons = [NSMutableArray arrayWithArray:@[
               // ASCTabViewTypeUnknown
               @{@"normal": @"", @"active": @""},
               // ASCTabViewTypeOpening
               @{@"normal": @"tab-icon_opening", @"active": @"tab-icon_opening"},
               // ASCTabViewTypeDocument
               @{@"normal": @"icon_tabs_de_inactive", @"active": @"icon_tabs_de_active"},
               // ASCTabViewTypeSpreadsheet
               @{@"normal": @"icon_tabs_se_inactive", @"active": @"icon_tabs_se_active"},
               // ASCTabViewTypePresentation
               @{@"normal": @"icon_tabs_pe_inactive", @"active": @"icon_tabs_pe_active"},
               // ASCTabViewTypePdf
               @{@"normal": @"icon_tabs_pdf_inactive", @"active": @"icon_tabs_pdf_active"},
               // ASCTabViewTypeDraw
               @{@"normal": @"icon_tabs_ve_inactive", @"active": @"icon_tabs_ve_active"},
               // ASCTabViewTypePortal
               @{@"normal": @"icon_tab_portal_active", @"active": @"icon_tab_portal_active"}
               ]
    ];

    [self refreshPortalTabIconset];

    ASCTabViewCell * tabCell = [[ASCTabViewCell alloc] initTextCell:self.title];
    [self setBordered:NO];
    [self setCell:tabCell];
    [self setState:NSControlStateValueOn];
//    [self.cell setImagePosition:NSImageLeft];
    [self.cell setBordered:NO];
        
    self.close = [[ASCButton alloc] initWithFrame:CGRectZero];
    [self.close setCell:[[ASCTabCloseButtonCell alloc] initTextCell:@""]];
    [self.close setBordered:NO];
    [self.close setTarget:self];
    [self.close setAction:@selector(onCloseTabButton:)];
    [self addSubview:self.close];
    
    [self.cell setCloseButton:self.close];
    
    [self.close setAutoresizingMask:NSViewMinXMargin | NSViewMaxYMargin];

    if ( CGRectGetWidth(self.frame) )
        [self setButtonCloseOrigin:self.frame];

    __weak __typeof__(self) weakSelf = self;
    tabCell.updateState = ^{
        if (weakSelf && weakSelf.close) {
            BOOL hiddenClose = true;

            if ([weakSelf state] || [weakSelf.close state]) {
                hiddenClose = false;
            } else {
                hiddenClose = !(tabCell.isHover || tabCell.isPressed);
            }

            [weakSelf.close setHidden:hiddenClose];
        }
    };

    [self setNeedsDisplay];
}

- (void)setFrame:(NSRect)frame {
    bool needUpdateCloseOrigin = self.frame.size.width != frame.size.width;

    [super setFrame:frame];

    if ( needUpdateCloseOrigin ) {
        [self setButtonCloseOrigin:frame];
    }
}

- (void)setState:(NSInteger)state {
    [super setState:state];
    
    NSString * iconName = (self.state)
        ? _icons[self.type][@"active"]
        : _icons[self.type][@"normal"];
    
    if ([iconName length] > 0) {
        self.image = [NSImage imageNamed:iconName];
    }

    ASCButton * closeButton = (ASCButton *)self.close;
    ASCButtonCell * buttonCell = [closeButton cell];

    if (buttonCell && closeButton) {
        ASCTabCloseButtonCell * closeButtonCell = [closeButton cell];

        if (closeButtonCell) {
            NSEvent *currentEvent = [NSApp currentEvent];
            [[closeButton cell] mouseExited:currentEvent];
            [closeButton setHidden:!(BOOL)self.state];
        }
    }

    [self setNeedsDisplay];

    if (_delegate && [_delegate respondsToSelector:@selector(tabDidUpdate:)]) {
        [_delegate tabDidUpdate:self];
    }
}

- (void)setType:(ASCTabViewType)type {
    _type = type;
    
    if (type > ASCTabViewTypeUnknown && type < [_icons count]) {
        [self refreshPortalTabIconset];
        NSString * iconName = (self.state)
            ? _icons[type][@"active"]
            : _icons[type][@"normal"];
        
        if ([iconName length] > 0) {
            self.image = [NSImage imageNamed:iconName];
        }
    }

    ASCTabViewCell * tabViewCell = (ASCTabViewCell *)self.cell;

    if (type == ASCTabViewTypePortal) {
        tabViewCell.activeColor     = [ASCThemesController currentThemeColor:btnPortalActiveBackgroundColor];
        tabViewCell.activeTextColor = [tabViewCell.activeColor isLight] ? NSColor.blackColor : NSColor.whiteColor;
    } else {
        tabViewCell.activeTextColor = [ASCThemesController currentThemeColor:tabActiveTextColor];

        if (type == ASCTabViewTypeDocument) {
            tabViewCell.activeColor =
            tabViewCell.clickColor  = [ASCThemesController currentThemeColor:tabWordActiveBackgroundColor];
        } else if (type == ASCTabViewTypeSpreadsheet) {
            tabViewCell.activeColor =
            tabViewCell.clickColor  = [ASCThemesController currentThemeColor:tabCellActiveBackgroundColor];
        } else if (type == ASCTabViewTypePresentation) {
            tabViewCell.activeColor =
            tabViewCell.clickColor  = [ASCThemesController currentThemeColor:tabSlideActiveBackgroundColor];
        } else if (type == ASCTabViewTypePdf) {
            tabViewCell.activeColor =
            tabViewCell.clickColor  = [ASCThemesController currentThemeColor:tabPdfActiveBackgroundColor];
        } else if (type == ASCTabViewTypeDraw) {
            tabViewCell.activeColor =
            tabViewCell.clickColor  = [ASCThemesController currentThemeColor:tabDrawActiveBackgroundColor];
        }
    }

    if (_delegate && [_delegate respondsToSelector:@selector(tabDidUpdate:)]) {
        [_delegate tabDidUpdate:self];
    }
}

- (void)setIsProcessing:(BOOL)isProcessing {
    if (_isProcessing != isProcessing) {
        _isProcessing = isProcessing;

        ASCTabViewCell * tabViewCell = self.cell;
        tabViewCell.isProcessing = _isProcessing;
        [self setNeedsDisplay];

        if (_delegate && [_delegate respondsToSelector:@selector(tabDidUpdate:)]) {
            [_delegate tabDidUpdate:self];
        }
    }
}

- (void)setButtonCloseOrigin:(NSRect)rect {
    CGFloat btnCloseOriginLeft = [self userInterfaceLayoutDirection] != NSUserInterfaceLayoutDirectionRightToLeft ?
                                        CGRectGetWidth(rect) - kASTabViewCloseButtonSize * 1.5 : kASTabViewCloseButtonSize / 1.5;

    [self.close setFrame:CGRectMake(btnCloseOriginLeft,
                                    kASTabViewCloseButtonSize / 1.5,
                                    kASTabViewCloseButtonSize,
                                    kASTabViewCloseButtonSize
                                    )];
}

- (void)setTitle:(NSString *)title {
    [super setTitle:title];
    
    originalTitle = title;
}

- (NSMutableDictionary *)params {
    if (nil == _params) {
        _params = [NSMutableDictionary dictionary];
    }
    
    return _params;
}

- (void)onCloseTabButton:(id)sender {
    if (_delegate && [_delegate respondsToSelector:@selector(tabDidClose:)]) {
        [_delegate tabDidClose:self];
    }
}

//- (void)setChanged:(BOOL)changed {
//    _changed = changed;
//
//    unichar l = [[super title] characterAtIndex:0];
//    if ( changed ) {
//        if ( l != '*' )
//            [super setTitle:[NSString stringWithFormat:@"*%@", originalTitle]];
//    } else {
//        if ( l == '*' )
//            [super setTitle:originalTitle];
//    }
//}

- (void)drawRect:(NSRect)dirtyRect {
//    [[NSColor greenColor] setFill];
//    NSRectFill(dirtyRect);
    
    [super drawRect:dirtyRect];
}



@end

//
//  NSLayoutConstraint+Extensions.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 29/03/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import "NSLayoutConstraint+Extensions.h"

@implementation NSLayoutConstraint (Extensions)

- (NSInteger)preciseConstant {
    return (NSInteger)(self.constant * [[NSScreen mainScreen] backingScaleFactor]);
}

- (void)setPreciseConstant:(NSInteger)preciseConstant {
    self.constant = (CGFloat)preciseConstant / [[NSScreen mainScreen] backingScaleFactor];
}

@end

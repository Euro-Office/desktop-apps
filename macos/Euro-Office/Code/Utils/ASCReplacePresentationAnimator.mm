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
//  ASCReplacePresentationAnimator.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 12/28/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCReplacePresentationAnimator.h"

@interface ASCReplacePresentationAnimator()

@end

@implementation ASCReplacePresentationAnimator

- (void)animatePresentationOfViewController:(NSViewController *)viewController fromViewController:(NSViewController *)fromViewController {
    NSWindow *window = fromViewController.view.window;

//    [window setContentViewController:viewController];
    
    fromViewController.view.hidden = YES;
    
    NSRect oldWindowFrame = window.frame;
    window.title = viewController.title;
    
    NSRect viewFrame = viewController.view.frame;
    viewFrame.size = viewController.view.fittingSize;
    
    NSArray *constraints = viewController.view.constraints;
    [viewController.view removeConstraints:constraints];
    
    NSRect windowFrame = [window frameRectForContentRect:viewFrame];
    windowFrame.origin = NSMakePoint(window.frame.origin.x + (NSWidth(oldWindowFrame) - NSWidth(windowFrame)) * 0.5, NSMaxY(window.frame) - NSHeight(windowFrame));
    
    viewController.view.wantsLayer = YES;
    viewController.view.layerContentsRedrawPolicy = NSViewLayerContentsRedrawOnSetNeedsDisplay;
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [[window animator] setFrame:windowFrame display:YES];
    } completionHandler:^{
        [window setContentViewController:viewController];
        [viewController.view addConstraints:constraints];
        
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [[viewController.view animator] setHidden:NO];
        } completionHandler:NULL];
    }];
}

- (void)animateDismissalOfViewController:(NSViewController *)viewController fromViewController:(NSViewController *)fromViewController {
    NSWindow *window = viewController.view.window;
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [[viewController.view animator] setAlphaValue:0];
    } completionHandler:^{
        [fromViewController.view setAlphaValue:0];
        [window setContentViewController:fromViewController];
        [[fromViewController.view animator] setAlphaValue:1.0];
    }];
}

@end

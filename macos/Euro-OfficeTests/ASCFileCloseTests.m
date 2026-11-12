// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

//
//  ASCFileCloseTests.m
//
//  File > Close in the editor sends editor:event {action: "file:close"}.
//  It must go through the same unsaved-changes check as the tab close button,
//  otherwise unsaved edits are dropped without a prompt.
//

#import <XCTest/XCTest.h>
#import <objc/runtime.h>

@interface ASCFileCloseTests : XCTestCase
@end

@implementation ASCFileCloseTests

- (NSViewController *)mainController {
    Class titleWindowClass = NSClassFromString(@"ASCTitleWindow");
    for (NSWindow *window in [NSApp windows]) {
        if ([window isKindOfClass:titleWindowClass]) {
            return window.contentViewController;
        }
    }
    return nil;
}

- (void)testFileCloseAsksBeforeClosingTabWithUnsavedChanges {
    NSViewController *controller = [self mainController];
    XCTAssertNotNil(controller, @"main window not found in test host");

    id tabsControl = [controller valueForKey:@"tabsControl"];
    NSView *tab = [[NSClassFromString(@"ASCTabView") alloc] initWithFrame:NSZeroRect];
    [tab setValue:@"-4242" forKey:@"uuid"];
    [[tabsControl valueForKey:@"tabs"] addObject:tab];
    NSTabView *tabView = [controller valueForKey:@"tabView"];
    NSTabViewItem *item = [[NSTabViewItem alloc] initWithIdentifier:@"-4242"];
    [tabView addTabViewItem:item];

    // Pretend the document has unsaved changes: the check asks the user and keeps the tab.
    __block BOOL asked = NO;
    SEL check = NSSelectorFromString(@"tabs:willRemovedTab:");
    Method method = class_getInstanceMethod([controller class], check);
    IMP original = method_setImplementation(method, imp_implementationWithBlock(^BOOL(id me, id control, id t) {
        asked = YES;
        return NO;
    }));

    [[NSNotificationCenter defaultCenter] postNotificationName:@"CEF_editorEvent"
                                                        object:nil
                                                      userInfo:@{@"viewId": @"-4242",
                                                                 @"data": @{@"action": @"file:close"}}];

    method_setImplementation(method, original);
    BOOL stillOpen = [[tabsControl valueForKey:@"tabs"] containsObject:tab];
    [[tabsControl valueForKey:@"tabs"] removeObject:tab];
    if ([tabView.tabViewItems containsObject:item]) {
        [tabView removeTabViewItem:item];
    }

    XCTAssertTrue(asked, @"file:close skipped the unsaved-changes check");
    XCTAssertTrue(stillOpen, @"file:close removed a tab with unsaved changes");
}

@end

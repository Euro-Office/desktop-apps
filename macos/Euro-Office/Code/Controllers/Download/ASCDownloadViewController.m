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
//  ASCDownloadViewController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 9/29/15.
//  Copyright © 2015 Ascensio System SIA. All rights reserved.
//

#import "ASCDownloadViewController.h"
#import "ASCDownloadController.h"
#import "ASCDownloadCellView.h"
#import "GTMNSString+HTML.h"

@interface ASCDownloadViewController() <NSTableViewDelegate, NSTableViewDataSource, ASCDownloadControllerDelegate, ASCDownloadCellViewDelegate>
@property (weak) IBOutlet NSTableView *tableView;
@property (weak) IBOutlet NSScrollView *tableScrollView;

@end

@implementation ASCDownloadViewController


- (void)viewDidLoad {
    [[[ASCDownloadController sharedInstance] multicastDelegate] addDelegate:self];
    [self updateViewConstraints];
}

- (void)viewDidDisappear {
    [[[ASCDownloadController sharedInstance] multicastDelegate] removeDelegate:self];
}

- (void)viewWillAppear {
    [self updatePopoverSize];
}

- (void)updatePopoverSize {
    int minHeight = self.tableView.rowHeight - 2; // 2px to hide cell border
    int maxHeight = self.tableView.rowHeight * 3;
    int newHeight = [[[ASCDownloadController sharedInstance] downloads] count] * self.tableView.rowHeight - 2; // 2px to hide cell border
    int width     = 270;
    
    [self.tableScrollView setHasVerticalScroller:(newHeight > maxHeight)];
    [self.tableScrollView setVerticalScrollElasticity:(newHeight > maxHeight) ? NSScrollElasticityAutomatic : NSScrollElasticityNone];
    
    self.preferredContentSize = NSMakeSize(width, MIN(MAX(minHeight, newHeight), maxHeight));
    
    if (self.popover) {
        NSSize oldSize = [[self.popover popoverWindow] frame].size;
        NSSize newSize = NSMakeSize(270, MIN(MAX(minHeight, newHeight), maxHeight));
        NSInteger delta = oldSize.height - newSize.height - self.popover.arrowWidth;
        
        if (delta > 0) {
            NSRect rect = NSOffsetRect([[self.popover popoverWindow] frame], 0, delta);
            [[[self.popover popoverWindow] contentView] setFrame:NSMakeRect(0, 0, width, newSize.height)];
            [[self.popover popoverWindow] setFrame:NSMakeRect(rect.origin.x, rect.origin.y, rect.size.width, newSize.height + self.popover.arrowWidth) display:YES];
        }
    }
}

#pragma mark -
#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [[[ASCDownloadController sharedInstance] downloads] count];
}

#pragma mark -
#pragma mark NSTableView Delegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    id download = [[[ASCDownloadController sharedInstance] downloads] objectAtIndex:row];

    ASCDownloadCellView * cellView  = [tableView makeViewWithIdentifier:@"ASCDownloadTableViewCellId" owner:self];
    cellView.textField.stringValue  = [download[@"name"] gtm_stringByUnescapingFromHTML];
    cellView.progress.doubleValue   = [download[@"percent"] doubleValue];
    cellView.uuid                   = download[@"idx"];
    cellView.delegate               = self;
    
    return cellView;
}

- (BOOL)selectionShouldChangeInTableView:(NSTableView *)tableView {
    return NO;
}

#pragma mark -
#pragma mark ASCDownloadCellView Delegate

- (void)onCancelButton:(ASCDownloadCellView *)cell {
    id download = [[ASCDownloadController sharedInstance] downloadWithId:cell.uuid];
    
    if (download) {
        download[@"canceled"] = @(YES);
    }
    
    [[ASCDownloadController sharedInstance] removeDownload:cell.uuid];
}

#pragma mark -
#pragma mark ASCDownloadController Delegate

- (void)downloadController:(ASCDownloadController *)controler didAddDownload:(id)download {
    [self.tableView reloadData];
    [self updatePopoverSize];
}

- (void)downloadController:(ASCDownloadController *)controler didRemovedDownload:(id)download {
    [self.tableView reloadData];
    
    if ([[[ASCDownloadController sharedInstance] downloads] count] < 1) {
        [self.popover closePopover:nil];
    } else {
        [self updatePopoverSize];
    }
}

- (void)downloadController:(ASCDownloadController *)controler didUpdatedDownload:(id)download {
    [self.tableView reloadData];
    [self updatePopoverSize];
}

@end

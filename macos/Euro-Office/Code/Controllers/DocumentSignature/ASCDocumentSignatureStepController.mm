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
//  ASCDocumentSignatureStepController.m
//  ONLYOFFICE
//
//  Created by Alexander Yuzhin on 16/04/2018.
//  Copyright © 2018 Ascensio System SIA. All rights reserved.
//

#import "ASCDocumentSignatureStepController.h"
#include "mac_application.h"
#include "CertificateCommon.h"
#import "NSView+Extensions.h"
#import "NSString+Extensions.h"
#import "NSAlert+SynchronousSheet.h"
#import "ASCDocSignController.h"
#import "ASCDocumentSignatureController.h"

@interface ASCDocumentSignatureStepController () <NSTextFieldDelegate>
@property (weak) IBOutlet NSSecureTextField *passwordField;
@end

@implementation ASCDocumentSignatureStepController

@synthesize navigationController = _navigationController;

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)viewDidAppear {
    [super viewDidAppear];

    if (self.passwordField) {
        self.passwordField.delegate = self;
        self.passwordField.focusRingType = NSFocusRingTypeNone;
        [self.passwordField becomeFirstResponder];
    }
}

#pragma mark - Internal

- (void)closeDialogWithResult:(BOOL)complete {
    if (CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager]) {
        if (CCefView * cefView = appManager->GetViewById((int)[[ASCDocSignController shared] cefId])) {
            NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent();
            NSEditorApi::CAscOpenSslData * pEventData = new NSEditorApi::CAscOpenSslData();

            if (pEvent && pEventData) {
                if (complete) {
                    pEventData->put_CertPath([ASCDocSignController.shared.signFilePath stdwstring]);
                    pEventData->put_CertPassword([ASCDocSignController.shared.signPassword stdwstring]);
                    pEventData->put_KeyPath([ASCDocSignController.shared.privateKeyFilePath stdwstring]);
                    pEventData->put_KeyPassword([ASCDocSignController.shared.privateKeyPassword stdwstring]);
                }

                pEvent->m_nType = ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE;
                pEvent->m_pData = pEventData;

                cefView->Apply(pEvent);
            }
        }
    }

    NSWindow * mainWindow = [[NSApplication sharedApplication] mainWindow];
    [mainWindow endSheet:[[self view] window]];
}

- (void)showErrorWithTitle:(NSString *)title subtitle:(NSString *)subtitle {
    NSAlert *alert = [NSAlert new];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:title];
    [alert setInformativeText:subtitle];
    [alert setAlertStyle:NSAlertStyleWarning];

    [alert runModalSheetForWindow:[[self view] window]];
}

- (BOOL)checkSignature {
    int nCertValue = NSOpenSSL::LoadCert([ASCDocSignController.shared.signFilePath stdwstring],
                                         [ASCDocSignController.shared.signPassword stdstring]);

    switch (nCertValue) {
        case OPEN_SSL_WARNING_ERR: {
            [self showErrorWithTitle:NSLocalizedString(@"Cannot open file of Digital Signature", nil)
                            subtitle:NSLocalizedString(@"ONLYOFFICE can not open the Digital Signature file. Try opening another file.", nil)];
            break;
        }
        case OPEN_SSL_WARNING_PASS: {
            if ([self.identifier isEqualToString:@"StepSignaturePasswordController"]) {
                return NO;
            } else {
                ASCDocumentSignatureStepController * controller = [self.storyboard instantiateControllerWithIdentifier:@"StepSignaturePasswordController"];
                if (controller) {
                    [self.navigationController pushViewController:controller animated:YES];
                }
            }
            return YES;
        }
        case OPEN_SSL_WARNING_OK: {
            ASCDocumentSignatureStepController * controller = [self.storyboard instantiateControllerWithIdentifier:@"StepPrivateKeyLoadController"];
            if (controller) {
                [self.navigationController pushViewController:controller animated:YES];
            }
            return YES;
        }
        case OPEN_SSL_WARNING_ALL_OK: {
            [self closeDialogWithResult:YES];
            return YES;
        }
        default:
            break;
    }

    return NO;
}

- (BOOL)checkPrivateKey {
    int nCertValue = NSOpenSSL::LoadKey([ASCDocSignController.shared.privateKeyFilePath stdwstring],
                                        [ASCDocSignController.shared.privateKeyPassword stdstring]);

    switch (nCertValue) {
        case OPEN_SSL_WARNING_ERR: {
            [self showErrorWithTitle:NSLocalizedString(@"Cannot open file of Private Key", nil)
                            subtitle:NSLocalizedString(@"ONLYOFFICE can not open the Private Key. Try opening another file.", nil)];
            break;
        }
        case OPEN_SSL_WARNING_PASS: {
            if ([self.identifier isEqualToString:@"StepPrivateKeyPasswordController"]) {
                return NO;
            } else {
                ASCDocumentSignatureStepController * controller = [self.storyboard instantiateControllerWithIdentifier:@"StepPrivateKeyPasswordController"];
                if (controller) {
                    [self.navigationController pushViewController:controller animated:YES];
                }
            }
            return YES;
        }
        case OPEN_SSL_WARNING_OK:
        case OPEN_SSL_WARNING_ALL_OK: {
            [self closeDialogWithResult:YES];
            return YES;
        }
        default:
            break;
    }

    return NO;
}

#pragma mark - Actions

- (IBAction)onCancel:(NSButton *)sender {
    [self closeDialogWithResult:NO];
}

- (IBAction)onSignatureLoad:(NSButton *)sender {
    NSOpenPanel * openPanel = [NSOpenPanel openPanel];

    openPanel.canChooseDirectories = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.canChooseFiles = YES;

    [openPanel beginSheetModalForWindow:[[self view] window] completionHandler:^(NSInteger result){
        [openPanel orderOut:self];

        if (result == NSFileHandlingPanelOKButton) {
            ASCDocSignController.shared.signFilePath = [NSString stringWithFormat:@"%@", [[openPanel URL] path]];
            [self checkSignature];
        }
    }];
}

- (IBAction)onSignaturePassword:(NSButton *)sender {
    if (self.passwordField) {
        ASCDocSignController.shared.signPassword = [self.passwordField stringValue];

        if (![self checkSignature]) {
            if (NSView * superview = self.passwordField.superview) {
                [superview shake];
            }
        }
    }
}

- (IBAction)onPrivateKeyLoad:(NSButton *)sender {
    NSOpenPanel * openPanel = [NSOpenPanel openPanel];

    openPanel.canChooseDirectories = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.canChooseFiles = YES;

    [openPanel beginSheetModalForWindow:[[self view] window] completionHandler:^(NSInteger result){
        [openPanel orderOut:self];

        if (result == NSFileHandlingPanelOKButton) {
            ASCDocSignController.shared.privateKeyFilePath = [NSString stringWithFormat:@"%@", [[openPanel URL] path]];
            [self checkPrivateKey];
        }
    }];
}

- (IBAction)onPrivateKeyPassword:(NSButton *)sender {
    if (self.passwordField) {
        ASCDocSignController.shared.privateKeyPassword = [self.passwordField stringValue];

        if (![self checkPrivateKey]) {
            if (NSView * superview = self.passwordField.superview) {
                [superview shake];
            }
        }
    }
}

@end


#pragma mark - NSTextField Delegate

@implementation ASCDocumentSignatureStepController (NSTextFieldDelegate)

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector {
    if (commandSelector == @selector(insertNewline:)) {
        if ([self.identifier isEqualToString:@"StepSignaturePasswordController"]) {
            [self onSignaturePassword:nil];
        } else if ([self.identifier isEqualToString:@"StepPrivateKeyPasswordController"]) {
            [self onPrivateKeyPassword:nil];
        }
        return YES;
    }
    return NO;
}

@end

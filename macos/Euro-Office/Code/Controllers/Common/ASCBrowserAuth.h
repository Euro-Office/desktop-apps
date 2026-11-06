// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

//
//  ASCBrowserAuth.h
//
//  macOS counterpart of CBrowserAuth (win-linux/src/cbrowserauth.h): native part
//  of the cloud login with the system browser. The login itself (e.g. Nextcloud
//  Login Flow v2) runs in the start page, see BrowserAuth in
//  loginpage/src/dialogconnect.js.
//
//  Keeps the resulting Authorization header value in the Keychain and sends it
//  with all requests to the cloud (CAscApplicationManager::SetPortalAuthorization).
//
//  Start page commands:
//    "auth:browser:open"  {url}                                  open the login page
//    "auth:browser:store" {portal, provider, type, login, authorization}
//  On logout the start page gets "auth:browser:revoke" {portal, type, authorization}
//  to invalidate the credentials on the server.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ASCBrowserAuth : NSObject

+ (instancetype)shared;

// authorize the clouds connected in previous sessions; call after the
// application manager is created
- (void)restore;

- (void)open:(NSString *)json;

// returns {portal, provider} of the cloud to open, nil for invalid input
- (nullable NSDictionary *)store:(NSString *)json;

// revokes and forgets the credentials; returns the "auth:browser:revoke"
// parameter for the start page, nil for clouds not connected this way
- (nullable NSString *)logout:(NSString *)portal;

@end

NS_ASSUME_NONNULL_END

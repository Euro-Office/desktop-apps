// SPDX-FileCopyrightText: 2026 Euro-Office contributors
// SPDX-License-Identifier: AGPL-3.0-only

#import "ASCBrowserAuth.h"
#import <AppKit/AppKit.h>
#import <Security/Security.h>
#import "applicationmanager.h"
#import "mac_application.h"
#import "NSString+Extensions.h"
#import "NSDictionary+Extensions.h"

static NSString * const kAccountsKey = @"browserAuthAccounts";

static NSString * normalizedPortal(NSString * portal) {
    while ([portal hasSuffix:@"/"])
        portal = [portal substringToIndex:portal.length - 1];
    return portal;
}

static BOOL isHttpUrl(NSString * url) {
    if (![url isKindOfClass:[NSString class]])
        return NO;
    NSURLComponents * components = [NSURLComponents componentsWithString:url];
    NSString * scheme = components.scheme.lowercaseString;
    return components && ([scheme isEqualToString:@"https"] || [scheme isEqualToString:@"http"]) && components.host.length > 0;
}

static NSString * stringValue(NSDictionary * json, NSString * key) {
    if (![json isKindOfClass:[NSDictionary class]])
        return @"";
    id value = json[key];
    return [value isKindOfClass:[NSString class]] ? value : @"";
}

#pragma mark - Keychain

static NSString * keychainService() {
    return [NSString stringWithFormat:@"%@.cloud", [[NSBundle mainBundle] bundleIdentifier] ?: @"Euro-Office"];
}

static NSMutableDictionary * keychainQuery(NSString * portal) {
    return [@{
        (__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService: keychainService(),
        (__bridge id)kSecAttrAccount: portal,
    } mutableCopy];
}

static void keychainRemove(NSString * portal) {
    SecItemDelete((__bridge CFDictionaryRef)keychainQuery(portal));
}

static BOOL keychainSave(NSString * portal, NSString * login, NSString * secret) {
    keychainRemove(portal);

    NSMutableDictionary * item = keychainQuery(portal);
    item[(__bridge id)kSecAttrLabel] = [NSString stringWithFormat:@"%@ (%@)", portal, login];
    item[(__bridge id)kSecValueData] = [secret dataUsingEncoding:NSUTF8StringEncoding];

    OSStatus status = SecItemAdd((__bridge CFDictionaryRef)item, NULL);
    if (status != errSecSuccess)
        NSLog(@"[BrowserAuth] keychain: can't save the credentials for %@: %d", portal, (int)status);
    return status == errSecSuccess;
}

static NSString * keychainLoad(NSString * portal) {
    NSMutableDictionary * query = keychainQuery(portal);
    query[(__bridge id)kSecReturnData] = @YES;
    query[(__bridge id)kSecMatchLimit] = (__bridge id)kSecMatchLimitOne;

    CFTypeRef result = NULL;
    if (SecItemCopyMatching((__bridge CFDictionaryRef)query, &result) != errSecSuccess || !result)
        return @"";

    NSData * data = (__bridge_transfer NSData *)result;
    return [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] ?: @"";
}

#pragma mark -

@interface ASCBrowserAuth ()
// portal -> {provider, type, login, authorization} authorized in this session,
// needed to revoke it on logout
@property (nonatomic) NSMutableDictionary<NSString *, NSDictionary *> * session;
@end

@implementation ASCBrowserAuth

+ (instancetype)shared {
    static ASCBrowserAuth * instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (instancetype)init {
    if (self = [super init]) {
        _session = [NSMutableDictionary dictionary];
        _startPageViewId = -1;
    }
    return self;
}

// accounts list in the user defaults, without secrets: portal -> {provider, type, login}
- (NSMutableDictionary<NSString *, NSDictionary *> *)accounts {
    NSDictionary * stored = [[NSUserDefaults standardUserDefaults] dictionaryForKey:kAccountsKey];
    return stored ? [stored mutableCopy] : [NSMutableDictionary dictionary];
}

- (void)setAccounts:(NSDictionary<NSString *, NSDictionary *> *)accounts {
    if (accounts.count)
        [[NSUserDefaults standardUserDefaults] setObject:accounts forKey:kAccountsKey];
    else
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:kAccountsKey];
}

- (void)restore {
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    NSMutableDictionary * accounts = [self accounts];

    for (NSString * portal in accounts.allKeys) {
        NSString * authorization = keychainLoad(portal);
        if (!authorization.length) {
            // keychain entry removed or not accessible: the user has to connect again
            [accounts removeObjectForKey:portal];
            continue;
        }

        NSMutableDictionary * account = [accounts[portal] mutableCopy];
        account[@"authorization"] = authorization;
        self.session[portal] = account;
        appManager->SetPortalAuthorization([portal stdwstring], [authorization stdwstring]);
    }

    [self setAccounts:accounts];
}

- (void)open:(NSString *)json {
    NSString * url = stringValue([json dictionary], @"url");
    if (!isHttpUrl(url))
        return;

    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:url]];
}

- (NSDictionary *)store:(NSString *)json {
    NSDictionary * obj = [json dictionary];
    NSString * portal = normalizedPortal(stringValue(obj, @"portal"));
    NSString * authorization = stringValue(obj, @"authorization");
    if (!isHttpUrl(portal) || !authorization.length)
        return nil;

    NSDictionary * account = @{
        @"provider": stringValue(obj, @"provider"),
        @"type": stringValue(obj, @"type"),
        @"login": stringValue(obj, @"login"),
    };

    NSMutableDictionary * sessionAccount = [account mutableCopy];
    sessionAccount[@"authorization"] = authorization;
    self.session[portal] = sessionAccount;

    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    appManager->SetPortalAuthorization([portal stdwstring], [authorization stdwstring]);

    if (keychainSave(portal, account[@"login"], authorization)) {
        NSMutableDictionary * accounts = [self accounts];
        accounts[portal] = account;
        [self setAccounts:accounts];
    }

    return @{@"portal": portal, @"provider": account[@"provider"]};
}

- (NSString *)logout:(NSString *)portal {
    portal = normalizedPortal(portal ?: @"");
    NSMutableDictionary * accounts = [self accounts];
    NSDictionary * account = self.session[portal];
    if (!accounts[portal] && !account)
        return nil;

    NSString * revoke = nil;
    NSString * authorization = account[@"authorization"];
    if (authorization.length) {
        revoke = [@{@"portal": portal, @"type": account[@"type"] ?: @"", @"authorization": authorization} jsonString];
    }

    [self.session removeObjectForKey:portal];
    CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
    appManager->SetPortalAuthorization([portal stdwstring], L"");
    keychainRemove(portal);
    [accounts removeObjectForKey:portal];
    [self setAccounts:accounts];

    return revoke;
}

@end

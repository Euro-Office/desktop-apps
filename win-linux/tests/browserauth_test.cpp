/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include <memory>
#include <QCoreApplication>
#include <QDeadlineTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>
#include "cbrowserauth.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "fakesdk.h"
#include "fakesecretservice.h"

// CBrowserAuth (src/cbrowserauth.cpp), the native part of the cloud login
// with the system browser: it keeps the Authorization header from the start
// page in the OS credential store (here the fake Secret Service, see
// fakesecretservice.h) and passes it to the SDK. Unit test build only
// (SCOPE unit), as it uses fakesdk.h.
//
// CBrowserAuth is a singleton; every test logs out of the portals it used.

namespace {

const QString PORTAL = "https://cloud.example.com";
const std::string SET_AUTHORIZATION = "CAscApplicationManager::SetPortalAuthorization";
const std::string START_PAGE_COMMANDS = "CAscApplicationManager::SetEventToAllMainWindows:command";

QString storeJson(const QString& portal, const QString& authorization = "Basic c2VjcmV0")
{
    return QJsonDocument(QJsonObject{{"portal", portal}, {"provider", "nextcloud"}, {"type", "nextcloud-v2"},
                                     {"login", "alice"}, {"authorization", authorization}}).toJson();
}

QJsonArray storedAccounts()
{
    GET_REGISTRY_USER(reg_user)
    return QJsonDocument::fromJson(reg_user.value("browserAuthAccounts").toByteArray()).array();
}

void setStoredAccounts(const QJsonArray& accounts)
{
    GET_REGISTRY_USER(reg_user)
    reg_user.setValue("browserAuthAccounts", QJsonDocument(accounts).toJson(QJsonDocument::Compact));
}

void sendCommand(const std::wstring& command, const QString& param)
{
    auto* data = new NSEditorApi::CAscExecCommand;
    data->put_Command(command);
    data->put_Param(param.toStdWString());
    auto* event = new NSEditorApi::CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND);
    event->m_pData = data;
    FakeSdk::sendEvent(event);

    // the app handles SDK events queued
    QCoreApplication::processEvents();
}

class BrowserAuthTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(FakeSecretService::busAvailable()) << "needs dbus-daemon for a private session bus";
        AscAppManager::getInstance();
        m_service = std::make_unique<FakeSecretService>();
        GET_REGISTRY_USER(reg_user)
        reg_user.remove("browserAuthAccounts");
        FakeSdk::reset();
    }

    void TearDown() override
    {
        for (const QString& portal : {PORTAL, QString("https://other.example.com")})
            CBrowserAuth::instance().logout(portal);
    }

    std::unique_ptr<FakeSecretService> m_service;
};

} // namespace

TEST_F(BrowserAuthTest, StoreAuthorizesThePortalAndKeepsTheSecretInTheKeyring)
{
    CBrowserAuth::instance().store(storeJson(PORTAL + "/"));

    EXPECT_EQ(FakeSdk::calls(SET_AUTHORIZATION),
              std::vector<std::wstring>{L"https://cloud.example.com Basic c2VjcmV0"});
    EXPECT_EQ(m_service->secretFor(PORTAL), "Basic c2VjcmV0");

    // the settings list the account, but not its secret
    const QJsonArray accounts = storedAccounts();
    ASSERT_EQ(accounts.size(), 1);
    EXPECT_EQ(accounts[0]["portal"].toString(), PORTAL);
    EXPECT_EQ(accounts[0]["provider"].toString(), "nextcloud");
    EXPECT_EQ(accounts[0]["login"].toString(), "alice");
    EXPECT_FALSE(accounts[0].toObject().contains("authorization"));
}

TEST_F(BrowserAuthTest, StoreIgnoresInvalidPortalsAndEmptyAuthorizations)
{
    CBrowserAuth::instance().store(storeJson("file:///etc/passwd"));
    CBrowserAuth::instance().store(storeJson("not a url"));
    CBrowserAuth::instance().store(storeJson(PORTAL, ""));
    CBrowserAuth::instance().store("not json");

    EXPECT_TRUE(FakeSdk::calls(SET_AUTHORIZATION).empty());
    EXPECT_TRUE(m_service->items.isEmpty());
    EXPECT_TRUE(storedAccounts().isEmpty());
}

TEST_F(BrowserAuthTest, StoreWithoutKeyringAuthorizesOnlyThisSession)
{
    m_service.reset();

    CBrowserAuth::instance().store(storeJson(PORTAL));

    EXPECT_EQ(FakeSdk::calls(SET_AUTHORIZATION).size(), 1u);
    EXPECT_TRUE(storedAccounts().isEmpty());
}

TEST_F(BrowserAuthTest, RestoreAuthorizesTheAccountsFoundInTheKeyring)
{
    setStoredAccounts({QJsonObject{{"portal", PORTAL}, {"provider", "nextcloud"}, {"type", "nextcloud-v2"}, {"login", "alice"}},
                       QJsonObject{{"portal", "https://other.example.com"}, {"provider", "nextcloud"}, {"login", "bob"}}});
    m_service->addItem(PORTAL, "Basic c2VjcmV0");

    CBrowserAuth::instance().restore();

    EXPECT_EQ(FakeSdk::calls(SET_AUTHORIZATION),
              std::vector<std::wstring>{L"https://cloud.example.com Basic c2VjcmV0"});
    // the other one has no secret anymore: the user has to connect again
    const QJsonArray accounts = storedAccounts();
    ASSERT_EQ(accounts.size(), 1);
    EXPECT_EQ(accounts[0]["portal"].toString(), PORTAL);
}

TEST_F(BrowserAuthTest, LogoutRevokesAndForgetsTheCredentials)
{
    CBrowserAuth::instance().store(storeJson(PORTAL));
    FakeSdk::clearCalls();

    CBrowserAuth::instance().logout(PORTAL + "/");

    const std::vector<std::wstring> commands = FakeSdk::calls(START_PAGE_COMMANDS);
    ASSERT_EQ(commands.size(), 1u);
    const std::wstring revoke = commands[0];
    EXPECT_EQ(revoke.rfind(L"auth:browser:revoke ", 0), 0u);
    const QJsonObject args = QJsonDocument::fromJson(
            QString::fromStdWString(revoke.substr(revoke.find(L' ') + 1)).toUtf8()).object();
    EXPECT_EQ(args["portal"].toString(), PORTAL);
    EXPECT_EQ(args["type"].toString(), "nextcloud-v2");
    EXPECT_EQ(args["authorization"].toString(), "Basic c2VjcmV0");

    EXPECT_EQ(FakeSdk::calls(SET_AUTHORIZATION), std::vector<std::wstring>{L"https://cloud.example.com "});
    EXPECT_TRUE(m_service->items.isEmpty());
    EXPECT_TRUE(storedAccounts().isEmpty());
}

TEST_F(BrowserAuthTest, LogoutOfAnAccountWithoutSecretOnlyForgetsIt)
{
    // listed in the settings, but restore() hasn't run
    setStoredAccounts({QJsonObject{{"portal", PORTAL}, {"provider", "nextcloud"}, {"login", "alice"}}});

    CBrowserAuth::instance().logout(PORTAL);

    EXPECT_TRUE(FakeSdk::calls(START_PAGE_COMMANDS).empty());
    EXPECT_EQ(FakeSdk::calls(SET_AUTHORIZATION).size(), 1u);
    EXPECT_TRUE(storedAccounts().isEmpty());
}

TEST_F(BrowserAuthTest, LogoutOfOtherPortalsDoesNothing)
{
    CBrowserAuth::instance().logout("https://unknown.example.com");

    EXPECT_TRUE(FakeSdk::calls(START_PAGE_COMMANDS).empty());
    EXPECT_TRUE(FakeSdk::calls(SET_AUTHORIZATION).empty());
}

TEST_F(BrowserAuthTest, OpenStartsTheBrowserWithoutTheAppsLibraries)
{
    // a fake xdg-open that writes down its argument and LD_LIBRARY_PATH
    QTemporaryDir bin;
    ASSERT_TRUE(bin.isValid());
    const QString out = bin.filePath("xdg-open.out");
    QFile script(bin.filePath("xdg-open"));
    ASSERT_TRUE(script.open(QIODevice::WriteOnly));
    script.write(QString("#!/bin/sh\nprintf '%s\\n%s\\n' \"$1\" \"${LD_LIBRARY_PATH-unset}\" > '%1.tmp'\nmv '%1.tmp' '%1'\n")
                 .arg(out).toUtf8());
    script.close();
    script.setPermissions(script.permissions() | QFile::ExeOwner);

    const QByteArray path = qgetenv("PATH"), libraryPath = qgetenv("LD_LIBRARY_PATH");
    qputenv("PATH", bin.path().toUtf8() + ":" + path);
    qputenv("LD_LIBRARY_PATH", "/opt/euro-office");

    CBrowserAuth::instance().open(R"({"url": "https://cloud.example.com/login/v2/flow/abc"})");

    qputenv("PATH", path);
    qputenv("LD_LIBRARY_PATH", libraryPath);

    QFile result(out);
    QDeadlineTimer deadline(10000);
    while (!result.exists() && !deadline.hasExpired())
        QThread::msleep(20);
    ASSERT_TRUE(result.open(QIODevice::ReadOnly)) << "xdg-open was not started";
    EXPECT_EQ(result.readAll(), "https://cloud.example.com/login/v2/flow/abc\nunset\n");
}

TEST_F(BrowserAuthTest, OpenIgnoresUrlsThatAreNotWebPages)
{
    QTemporaryDir bin;
    ASSERT_TRUE(bin.isValid());
    QFile script(bin.filePath("xdg-open"));
    ASSERT_TRUE(script.open(QIODevice::WriteOnly));
    script.write(QString("#!/bin/sh\ntouch '%1'\n").arg(bin.filePath("started")).toUtf8());
    script.close();
    script.setPermissions(script.permissions() | QFile::ExeOwner);

    const QByteArray path = qgetenv("PATH");
    qputenv("PATH", bin.path().toUtf8() + ":" + path);
    for (const char* json : {R"({"url": "file:///etc/passwd"})", R"({"url": "https://"})", R"({})", "not json"})
        CBrowserAuth::instance().open(json);
    qputenv("PATH", path);

    QThread::msleep(200);
    EXPECT_FALSE(QFile::exists(bin.filePath("started")));
}

TEST_F(BrowserAuthTest, StartPageCommandsReachBrowserAuth)
{
    sendCommand(L"auth:browser:store", storeJson(PORTAL));
    EXPECT_EQ(FakeSdk::calls(SET_AUTHORIZATION).size(), 1u);

    // nothing to open: only checks that the command is taken
    sendCommand(L"auth:browser:open", R"({"url": "ftp://cloud.example.com"})");
}

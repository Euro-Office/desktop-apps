/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include <memory>
#include "ccredentialstore.h"
#include "fakesecretservice.h"

// CCredentialStore (src/ccredentialstore.cpp) on Linux: secrets kept in the
// Secret Service, here an in-memory one on a private session bus (see
// fakesecretservice.h).

namespace {

class CredentialStoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(FakeSecretService::busAvailable()) << "needs dbus-daemon for a private session bus";
        m_service = std::make_unique<FakeSecretService>();
    }

    std::unique_ptr<FakeSecretService> m_service;
};

} // namespace

TEST_F(CredentialStoreTest, SavesAndLoadsASecret)
{
    EXPECT_TRUE(CCredentialStore::save("https://cloud.example.com", "alice", "Basic c2VjcmV0"));

    EXPECT_EQ(m_service->secretFor("https://cloud.example.com"), "Basic c2VjcmV0");
    EXPECT_EQ(CCredentialStore::load("https://cloud.example.com"), "Basic c2VjcmV0");
}

TEST_F(CredentialStoreTest, LabelsTheItemWithKeyAndLogin)
{
    ASSERT_TRUE(CCredentialStore::save("https://cloud.example.com", "alice", "secret"));

    ASSERT_EQ(m_service->items.size(), 1);
    EXPECT_EQ(m_service->items.first().label, "Euro-Office: https://cloud.example.com (alice)");
}

TEST_F(CredentialStoreTest, SavingAgainReplacesTheSecret)
{
    ASSERT_TRUE(CCredentialStore::save("https://cloud.example.com", "alice", "old"));
    ASSERT_TRUE(CCredentialStore::save("https://cloud.example.com", "alice", "new"));

    EXPECT_EQ(m_service->items.size(), 1);
    EXPECT_EQ(CCredentialStore::load("https://cloud.example.com"), "new");
}

TEST_F(CredentialStoreTest, LoadsNothingForAnUnknownKey)
{
    m_service->addItem("https://other.example.com", "secret");

    EXPECT_TRUE(CCredentialStore::load("https://cloud.example.com").isEmpty());
}

TEST_F(CredentialStoreTest, RemovesTheSecret)
{
    m_service->addItem("https://cloud.example.com", "secret");
    m_service->addItem("https://other.example.com", "other");

    CCredentialStore::remove("https://cloud.example.com");

    EXPECT_TRUE(m_service->secretFor("https://cloud.example.com").isEmpty());
    EXPECT_EQ(m_service->secretFor("https://other.example.com"), "other");
}

TEST_F(CredentialStoreTest, UnlocksALockedKeyringWithAPrompt)
{
    m_service->collectionLocked = true;

    EXPECT_TRUE(CCredentialStore::save("https://cloud.example.com", "alice", "secret"));
    EXPECT_EQ(m_service->promptCount, 1);
    EXPECT_EQ(m_service->secretFor("https://cloud.example.com"), "secret");
}

TEST_F(CredentialStoreTest, UnlocksALockedItemToLoadIt)
{
    m_service->addItem("https://cloud.example.com", "secret", true);

    EXPECT_EQ(CCredentialStore::load("https://cloud.example.com"), "secret");
    EXPECT_EQ(m_service->promptCount, 1);
}

TEST_F(CredentialStoreTest, RunsThePromptsOfCreateAndDelete)
{
    m_service->promptOnCreate = true;
    m_service->promptOnDelete = true;

    EXPECT_TRUE(CCredentialStore::save("https://cloud.example.com", "alice", "secret"));
    EXPECT_EQ(m_service->secretFor("https://cloud.example.com"), "secret");

    CCredentialStore::remove("https://cloud.example.com");
    EXPECT_TRUE(m_service->items.isEmpty());
    EXPECT_EQ(m_service->promptCount, 2);
}

TEST_F(CredentialStoreTest, FailsWhenTheUserDismissesThePrompt)
{
    m_service->collectionLocked = true;
    m_service->dismissPrompts = true;

    EXPECT_FALSE(CCredentialStore::save("https://cloud.example.com", "alice", "secret"));
    EXPECT_TRUE(m_service->items.isEmpty());
}

TEST_F(CredentialStoreTest, LoadsNothingFromALockedItemTheUserKeepsLocked)
{
    m_service->addItem("https://cloud.example.com", "secret", true);
    m_service->dismissPrompts = true;

    EXPECT_TRUE(CCredentialStore::load("https://cloud.example.com").isEmpty());
}

TEST_F(CredentialStoreTest, FailsWithoutADefaultCollection)
{
    m_service->hasDefaultCollection = false;

    EXPECT_FALSE(CCredentialStore::save("https://cloud.example.com", "alice", "secret"));
}

TEST_F(CredentialStoreTest, FailsWhenTheServiceReturnsErrors)
{
    m_service->addItem("https://cloud.example.com", "secret");

    for (const QString method : {"ReadAlias", "Unlock", "OpenSession", "CreateItem"}) {
        m_service->failingMethods = QStringList{method};
        EXPECT_FALSE(CCredentialStore::save("https://new.example.com", "alice", "secret")) << method.toStdString();
    }
    for (const QString method : {"SearchItems", "OpenSession", "GetSecrets"}) {
        m_service->failingMethods = QStringList{method};
        EXPECT_TRUE(CCredentialStore::load("https://cloud.example.com").isEmpty()) << method.toStdString();
    }

    m_service->failingMethods = QStringList{"Delete"};
    CCredentialStore::remove("https://cloud.example.com");
    EXPECT_EQ(m_service->secretFor("https://cloud.example.com"), "secret");
}

TEST_F(CredentialStoreTest, DoesNothingWithoutASecretService)
{
    m_service.reset();

    EXPECT_FALSE(CCredentialStore::save("https://cloud.example.com", "alice", "secret"));
    EXPECT_TRUE(CCredentialStore::load("https://cloud.example.com").isEmpty());
    CCredentialStore::remove("https://cloud.example.com");
}

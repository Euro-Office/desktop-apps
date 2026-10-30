/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef FAKESECRETSERVICE_H
#define FAKESECRETSERVICE_H

#include <QByteArray>
#include <QDBusVirtualObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <functional>

// An in-memory Secret Service (org.freedesktop.secrets) for the tests of the
// OS credential store on Linux (src/ccredentialstore.cpp).
//
// Linking fakesecretservice.cpp starts a private D-Bus session bus for the
// test binary and points XDG_CONFIG_HOME to a temporary directory before
// main() runs, so the tests touch neither the user's keyring nor the app's
// settings. The service lives in the test process: the app's calls to it are
// delivered directly by Qt, and prompts (e.g. "unlock the keyring") are
// answered through the bus, as by a real keyring.
class FakeSecretService : public QDBusVirtualObject
{
public:
    struct Item
    {
        QMap<QString, QString> attributes;
        QString label;
        QByteArray secret;
        bool locked = false;
    };

    // Registers the service on the session bus, unregisters it when destroyed
    FakeSecretService();
    ~FakeSecretService() override;

    // false if the private session bus could not be started
    static bool busAvailable();

    // Stored items by their object path
    QMap<QString, Item> items;

    // How the fake behaves; the defaults are those of an unlocked keyring
    bool hasDefaultCollection = true;
    bool collectionLocked = false;
    bool dismissPrompts = false;         // the user cancels every prompt
    bool promptOnCreate = false;         // CreateItem needs a prompt
    bool promptOnDelete = false;         // Item.Delete needs a prompt
    QStringList failingMethods;          // methods that return an error

    // Prompts the app ran
    int promptCount = 0;

    // Secret of the item with the app's attributes for `key`, empty if none
    QByteArray secretFor(const QString& key) const;
    void addItem(const QString& key, const QByteArray& secret, bool locked = false);

    QString introspect(const QString& path) const override;
    bool handleMessage(const QDBusMessage& message, const QDBusConnection& connection) override;

private:
    QString newPrompt(bool unlock, const QList<QString>& objects);
    QList<QString> search(const QMap<QString, QString>& attributes) const;

    int m_nextId = 1;
    // pending prompts: path -> objects they unlock, and what they do else
    QMap<QString, QList<QString>> m_prompts;
    QMap<QString, std::function<void()>> m_pending;
};

#endif // FAKESECRETSERVICE_H

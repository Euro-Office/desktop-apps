#ifndef CCREDENTIALSTORE_H
#define CCREDENTIALSTORE_H

#include <QString>

// Stores secrets in the OS credential store: Windows Credential Manager,
// the Secret Service (GNOME Keyring, KWallet) on Linux. Secrets are never
// written to disk by the app itself; if no store is available, save() fails.
namespace CCredentialStore
{
    bool save(const QString& key, const QString& login, const QString& secret);
    QString load(const QString& key);
    void remove(const QString& key);
}

#endif // CCREDENTIALSTORE_H

#ifndef CBROWSERAUTH_H
#define CBROWSERAUTH_H

#include <QObject>
#include <QMap>

// Native part of the cloud login with the system browser. The login itself
// (e.g. Nextcloud Login Flow v2) runs in the start page, see BrowserAuth in
// loginpage/src/dialogconnect.js; its requests go through CEF, not Qt Network,
// because the OpenSSL linked into the app clashes with the one Qt loads.
//
// This class keeps the resulting Authorization header value in the OS
// credential store and sends it with all requests to the cloud
// (CAscApplicationManager::SetPortalAuthorization).
//
// Start page commands:
//   "auth:browser:open"  {url}                                  open the login page
//   "auth:browser:store" {portal, provider, type, login, authorization}
// On logout the start page gets "auth:browser:revoke" {portal, type, authorization}
// to invalidate the credentials on the server.
class CBrowserAuth : public QObject
{
    Q_OBJECT
public:
    static CBrowserAuth& instance();

    // authorize the clouds connected in previous sessions
    void restore();

    void open(const QString& json);
    void store(const QString& json);

    // revoke the credentials and forget them; no-op for clouds not connected this way
    void logout(const QString& portal);

private:
    explicit CBrowserAuth(QObject * parent = nullptr);

    struct CAccount {
        QString provider;
        QString type;
        QString login;
        QString authorization;  // only kept in memory, see m_session
    };

    // accounts list in the settings, without secrets
    QMap<QString, CAccount> accounts() const;
    void setAccounts(const QMap<QString, CAccount>& accounts);

    // portal <-> account authorized in this session, needed to revoke it on logout
    QMap<QString, CAccount> m_session;
};

#endif // CBROWSERAUTH_H

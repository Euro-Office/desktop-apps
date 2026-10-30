#include "cbrowserauth.h"
#include "ccredentialstore.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUrl>
#ifdef __linux
# include <QProcess>
#else
# include <QDesktopServices>
#endif

#define BROWSER_AUTH_ACCOUNTS_KEY "browserAuthAccounts"

namespace {
    QString normalized(QString portal)
    {
        while ( portal.endsWith('/') )
            portal.chop(1);
        return portal;
    }

    bool isHttpUrl(const QString& url)
    {
        const QUrl _url(url);
        return _url.isValid() && (_url.scheme() == "https" || _url.scheme() == "http") && !_url.host().isEmpty();
    }
}

CBrowserAuth& CBrowserAuth::instance()
{
    static CBrowserAuth _instance;
    return _instance;
}

CBrowserAuth::CBrowserAuth(QObject * parent)
    : QObject(parent)
{}

QMap<QString, CBrowserAuth::CAccount> CBrowserAuth::accounts() const
{
    GET_REGISTRY_USER(reg_user)
    QMap<QString, CAccount> out;
    const QJsonArray arr = QJsonDocument::fromJson(reg_user.value(BROWSER_AUTH_ACCOUNTS_KEY).toByteArray()).array();
    for (const QJsonValue& v : arr)
        out[v["portal"].toString()] = CAccount{v["provider"].toString(), v["type"].toString(), v["login"].toString(), QString()};
    return out;
}

void CBrowserAuth::setAccounts(const QMap<QString, CAccount>& accounts)
{
    QJsonArray arr;
    for (auto it = accounts.cbegin(); it != accounts.cend(); ++it)
        arr.append(QJsonObject{{"portal", it.key()}, {"provider", it->provider}, {"type", it->type}, {"login", it->login}});

    GET_REGISTRY_USER(reg_user)
    if ( arr.isEmpty() )
        reg_user.remove(BROWSER_AUTH_ACCOUNTS_KEY);
    else reg_user.setValue(BROWSER_AUTH_ACCOUNTS_KEY, QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

void CBrowserAuth::restore()
{
    QMap<QString, CAccount> list = accounts();
    for (auto it = list.begin(); it != list.end();) {
        it->authorization = CCredentialStore::load(it.key());
        if ( it->authorization.isEmpty() ) {
            // keyring unavailable or entry removed: the user has to connect again
            it = list.erase(it);
        } else {
            m_session[it.key()] = *it;
            AscAppManager::getInstance().SetPortalAuthorization(it.key().toStdWString(), it->authorization.toStdWString());
            ++it;
        }
    }
    setAccounts(list);
}

void CBrowserAuth::open(const QString& json)
{
    const QString url = QJsonDocument::fromJson(json.toUtf8()).object()["url"].toString();
    if ( !isHttpUrl(url) )
        return;

#ifdef __linux
    // no shell: the url comes from the server
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("LD_LIBRARY_PATH");
    env.remove("LD_PRELOAD");
    process.setProcessEnvironment(env);
    process.setProgram("xdg-open");
    process.setArguments({url});
    process.startDetached();
#else
    QDesktopServices::openUrl(QUrl(url));
#endif
}

void CBrowserAuth::store(const QString& json)
{
    const QJsonObject obj = QJsonDocument::fromJson(json.toUtf8()).object();
    const QString portal = normalized(obj["portal"].toString());
    const CAccount account{obj["provider"].toString(), obj["type"].toString(), obj["login"].toString(), obj["authorization"].toString()};
    if ( !isHttpUrl(portal) || account.authorization.isEmpty() )
        return;

    m_session[portal] = account;
    AscAppManager::getInstance().SetPortalAuthorization(portal.toStdWString(), account.authorization.toStdWString());
    if ( CCredentialStore::save(portal, account.login, account.authorization) ) {
        QMap<QString, CAccount> list = accounts();
        list[portal] = account;
        setAccounts(list);
    }

    // the cloud page is now logged in by the Authorization header and reports
    // "portal:login" from there, as with the in-app login
    QMetaObject::invokeMethod(AscAppManager::mainWindow(), "onOutsideAuth", Qt::QueuedConnection,
            Q_ARG(QString, Utils::stringifyJson(QJsonObject{{"portal", portal}, {"type", "outer"}, {"provider", account.provider}, {"entrypage", ""}})));
}

void CBrowserAuth::logout(const QString& _portal)
{
    const QString portal = normalized(_portal);
    QMap<QString, CAccount> list = accounts();
    if ( !list.contains(portal) && !m_session.contains(portal) )
        return;

    const CAccount account = m_session.value(portal);
    if ( !account.authorization.isEmpty() ) {
        AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"auth:browser:revoke",
                Utils::stringifyJson(QJsonObject{{"portal", portal}, {"type", account.type}, {"authorization", account.authorization}}).toStdWString());
    }

    m_session.remove(portal);
    AscAppManager::getInstance().SetPortalAuthorization(portal.toStdWString(), L"");
    CCredentialStore::remove(portal);
    list.remove(portal);
    setAccounts(list);
}

#include "ccredentialstore.h"

#ifdef _WIN32
# include <windows.h>
# include <wincred.h>
#else
# include <QDBusConnection>
# include <QDBusInterface>
# include <QDBusReply>
# include <QDBusMetaType>
# include <QDBusObjectPath>
# include <QDBusVariant>
# include <QEventLoop>
# include <QTimer>
# include <QMap>
# include <functional>
#endif

#ifdef _WIN32

namespace {
    std::wstring targetName(const QString& key)
    {
        return (QStringLiteral("Euro-Office/") + key).toStdWString();
    }
}

bool CCredentialStore::save(const QString& key, const QString& login, const QString& secret)
{
    const std::wstring target = targetName(key);
    const std::wstring user = login.toStdWString();
    QByteArray blob = secret.toUtf8();

    CREDENTIALW cred{};
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = const_cast<LPWSTR>(target.c_str());
    cred.UserName = const_cast<LPWSTR>(user.c_str());
    cred.CredentialBlobSize = (DWORD)blob.size();
    cred.CredentialBlob = reinterpret_cast<LPBYTE>(blob.data());
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    return CredWriteW(&cred, 0) == TRUE;
}

QString CCredentialStore::load(const QString& key)
{
    PCREDENTIALW cred = nullptr;
    if ( !CredReadW(targetName(key).c_str(), CRED_TYPE_GENERIC, 0, &cred) )
        return QString();

    QString secret = QString::fromUtf8(reinterpret_cast<const char*>(cred->CredentialBlob), (int)cred->CredentialBlobSize);
    CredFree(cred);
    return secret;
}

void CCredentialStore::remove(const QString& key)
{
    CredDeleteW(targetName(key).c_str(), CRED_TYPE_GENERIC, 0);
}

#else

// Secret Service API: https://specifications.freedesktop.org/secret-service/
struct CSecret
{
    QDBusObjectPath session;
    QByteArray parameters;
    QByteArray value;
    QString contentType;
};
typedef QMap<QDBusObjectPath, CSecret> CSecretMap;
typedef QMap<QString, QString> CStringMap;
Q_DECLARE_METATYPE(CSecret)
Q_DECLARE_METATYPE(CSecretMap)
Q_DECLARE_METATYPE(CStringMap)

QDBusArgument& operator<<(QDBusArgument& arg, const CSecret& s)
{
    arg.beginStructure();
    arg << s.session << s.parameters << s.value << s.contentType;
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, CSecret& s)
{
    arg.beginStructure();
    arg >> s.session >> s.parameters >> s.value >> s.contentType;
    arg.endStructure();
    return arg;
}

// receives the Prompt.Completed signal for runPrompt()
class PromptWaiter : public QObject
{
    Q_OBJECT
public:
    explicit PromptWaiter(std::function<void(bool)> done) : m_done(std::move(done)) {}
public slots:
    void completed(bool dismissed, QDBusVariant) { m_done(dismissed); }
private:
    std::function<void(bool)> m_done;
};

namespace {
    const QString SS_SERVICE = QStringLiteral("org.freedesktop.secrets");
    const QString SS_PATH = QStringLiteral("/org/freedesktop/secrets");
    const QString SS_IFACE_SERVICE = QStringLiteral("org.freedesktop.Secret.Service");
    const QString SS_IFACE_COLLECTION = QStringLiteral("org.freedesktop.Secret.Collection");
    const QString SS_IFACE_ITEM = QStringLiteral("org.freedesktop.Secret.Item");
    const QString SS_IFACE_PROMPT = QStringLiteral("org.freedesktop.Secret.Prompt");
    const QString NO_PROMPT = QStringLiteral("/");

    void registerTypes()
    {
        static bool registered = false;
        if ( !registered ) {
            qDBusRegisterMetaType<CSecret>();
            qDBusRegisterMetaType<CSecretMap>();
            qDBusRegisterMetaType<CStringMap>();
            registered = true;
        }
    }

    CStringMap attributes(const QString& key)
    {
        return CStringMap{{"application", "eurooffice-desktopeditors"}, {"key", key}};
    }

    // Runs a prompt (e.g. "unlock keyring") and waits until the user answers it.
    bool runPrompt(const QDBusObjectPath& prompt)
    {
        if ( prompt.path() == NO_PROMPT )
            return true;

        QDBusConnection bus = QDBusConnection::sessionBus();
        QEventLoop loop;
        bool dismissed = true;
        PromptWaiter waiter([&](bool d) { dismissed = d; loop.quit(); });
        bus.connect(SS_SERVICE, prompt.path(), SS_IFACE_PROMPT, "Completed", &waiter, SLOT(completed(bool,QDBusVariant)));

        QDBusInterface iface(SS_SERVICE, prompt.path(), SS_IFACE_PROMPT, bus);
        iface.call("Prompt", QString());
        QTimer::singleShot(120000, &loop, &QEventLoop::quit);
        loop.exec();

        bus.disconnect(SS_SERVICE, prompt.path(), SS_IFACE_PROMPT, "Completed", &waiter, SLOT(completed(bool,QDBusVariant)));
        return !dismissed;
    }

    QDBusObjectPath openSession(QDBusInterface& service)
    {
        QDBusMessage reply = service.call("OpenSession", QString("plain"), QVariant::fromValue(QDBusVariant(QString())));
        if ( reply.type() != QDBusMessage::ReplyMessage || reply.arguments().size() < 2 )
            return QDBusObjectPath();
        return reply.arguments().at(1).value<QDBusObjectPath>();
    }

    bool unlock(QDBusInterface& service, const QList<QDBusObjectPath>& objects)
    {
        QDBusMessage reply = service.call("Unlock", QVariant::fromValue(objects));
        if ( reply.type() != QDBusMessage::ReplyMessage || reply.arguments().size() < 2 )
            return false;
        return runPrompt(reply.arguments().at(1).value<QDBusObjectPath>());
    }

    QList<QDBusObjectPath> searchItems(QDBusInterface& service, const QString& key)
    {
        QDBusMessage reply = service.call("SearchItems", QVariant::fromValue(attributes(key)));
        if ( reply.type() != QDBusMessage::ReplyMessage || reply.arguments().size() < 2 )
            return {};

        QList<QDBusObjectPath> unlocked = qdbus_cast<QList<QDBusObjectPath>>(reply.arguments().at(0));
        QList<QDBusObjectPath> locked = qdbus_cast<QList<QDBusObjectPath>>(reply.arguments().at(1));
        if ( !locked.isEmpty() && unlock(service, locked) )
            unlocked += locked;
        return unlocked;
    }
}

bool CCredentialStore::save(const QString& key, const QString& login, const QString& secret)
{
    registerTypes();
    QDBusInterface service(SS_SERVICE, SS_PATH, SS_IFACE_SERVICE, QDBusConnection::sessionBus());
    if ( !service.isValid() )
        return false;

    QDBusReply<QDBusObjectPath> alias = service.call("ReadAlias", QString("default"));
    if ( !alias.isValid() || alias.value().path() == NO_PROMPT )
        return false;

    const QDBusObjectPath collection = alias.value();
    if ( !unlock(service, {collection}) )
        return false;

    CSecret s;
    s.session = openSession(service);
    s.value = secret.toUtf8();
    s.contentType = QStringLiteral("text/plain");
    if ( s.session.path().isEmpty() )
        return false;

    QVariantMap props;
    props["org.freedesktop.Secret.Item.Label"] = QStringLiteral("Euro-Office: %1 (%2)").arg(key, login);
    props["org.freedesktop.Secret.Item.Attributes"] = QVariant::fromValue(attributes(key));

    QDBusInterface coll(SS_SERVICE, collection.path(), SS_IFACE_COLLECTION, QDBusConnection::sessionBus());
    QDBusMessage reply = coll.call("CreateItem", props, QVariant::fromValue(s), true);
    if ( reply.type() != QDBusMessage::ReplyMessage || reply.arguments().size() < 2 )
        return false;
    return runPrompt(reply.arguments().at(1).value<QDBusObjectPath>());
}

QString CCredentialStore::load(const QString& key)
{
    registerTypes();
    QDBusInterface service(SS_SERVICE, SS_PATH, SS_IFACE_SERVICE, QDBusConnection::sessionBus());
    if ( !service.isValid() )
        return QString();

    const QList<QDBusObjectPath> items = searchItems(service, key);
    const QDBusObjectPath session = openSession(service);
    if ( items.isEmpty() || session.path().isEmpty() )
        return QString();

    QDBusMessage reply = service.call("GetSecrets", QVariant::fromValue(items), QVariant::fromValue(session));
    if ( reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty() )
        return QString();

    const CSecretMap secrets = qdbus_cast<CSecretMap>(reply.arguments().at(0));
    return secrets.isEmpty() ? QString() : QString::fromUtf8(secrets.first().value);
}

void CCredentialStore::remove(const QString& key)
{
    registerTypes();
    QDBusInterface service(SS_SERVICE, SS_PATH, SS_IFACE_SERVICE, QDBusConnection::sessionBus());
    if ( !service.isValid() )
        return;

    for ( const QDBusObjectPath& item : searchItems(service, key) ) {
        QDBusInterface iface(SS_SERVICE, item.path(), SS_IFACE_ITEM, QDBusConnection::sessionBus());
        QDBusReply<QDBusObjectPath> prompt = iface.call("Delete");
        if ( prompt.isValid() )
            runPrompt(prompt.value());
    }
}

#include "ccredentialstore.moc"

#endif

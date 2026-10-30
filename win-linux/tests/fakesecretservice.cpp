/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "fakesecretservice.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QDir>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <string>

// The Secret struct of the Secret Service API: (oayays)
struct FakeSecret
{
    QDBusObjectPath session;
    QByteArray parameters;
    QByteArray value;
    QString contentType;
};
typedef QMap<QDBusObjectPath, FakeSecret> FakeSecretMap;
typedef QMap<QString, QString> FakeStringMap;
Q_DECLARE_METATYPE(FakeSecret)
Q_DECLARE_METATYPE(FakeSecretMap)

QDBusArgument& operator<<(QDBusArgument& arg, const FakeSecret& s)
{
    arg.beginStructure();
    arg << s.session << s.parameters << s.value << s.contentType;
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, FakeSecret& s)
{
    arg.beginStructure();
    arg >> s.session >> s.parameters >> s.value >> s.contentType;
    arg.endStructure();
    return arg;
}

namespace {

const QString SERVICE = QStringLiteral("org.freedesktop.secrets");
const QString ROOT = QStringLiteral("/org/freedesktop/secrets");
const QString COLLECTION = ROOT + QStringLiteral("/collection/login");
const QString NO_PROMPT = QStringLiteral("/");

// A session bus of its own for the test binary, started before main() so Qt
// never connects to the user's one, and an empty settings directory.
class PrivateBus
{
public:
    PrivateBus()
    {
        const char* tmp = std::getenv("TMPDIR");
        std::string dir = std::string(tmp && *tmp ? tmp : "/tmp") + "/eo-test-XXXXXX";
        if (mkdtemp(&dir[0])) {
            m_configDir = dir;
            setenv("XDG_CONFIG_HOME", dir.c_str(), 1);
        }

        // never fall back to the user's bus
        setenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/nonexistent", 1);
        m_pipe = popen("exec dbus-daemon --session --nofork --print-address=1 --print-pid=1 2>/dev/null", "r");
        char address[512], pid[32];
        if (m_pipe && fgets(address, sizeof(address), m_pipe) && fgets(pid, sizeof(pid), m_pipe)) {
            std::string a(address);
            a.erase(a.find_last_not_of("\r\n") + 1);
            setenv("DBUS_SESSION_BUS_ADDRESS", a.c_str(), 1);
            m_pid = std::atoi(pid);
        }
    }

    ~PrivateBus()
    {
        if (m_pid > 0)
            kill(m_pid, SIGTERM);
        if (m_pipe)
            pclose(m_pipe);
        if (!m_configDir.empty())
            QDir(QString::fromStdString(m_configDir)).removeRecursively();
    }

    bool available() const { return m_pid > 0; }

private:
    FILE* m_pipe = nullptr;
    int m_pid = 0;
    std::string m_configDir;
};

PrivateBus s_bus;

QString interfaceOf(const QString& path)
{
    if (path == ROOT)
        return "org.freedesktop.Secret.Service";
    if (path == COLLECTION)
        return "org.freedesktop.Secret.Collection";
    if (path.startsWith(ROOT + "/prompt/"))
        return "org.freedesktop.Secret.Prompt";
    return "org.freedesktop.Secret.Item";
}

QVariant pathList(const QList<QString>& paths)
{
    QList<QDBusObjectPath> out;
    for (const QString& p : paths)
        out.append(QDBusObjectPath(p));
    return QVariant::fromValue(out);
}

} // namespace

FakeSecretService::FakeSecretService()
{
    qDBusRegisterMetaType<FakeSecret>();
    qDBusRegisterMetaType<FakeSecretMap>();
    qDBusRegisterMetaType<FakeStringMap>();

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerVirtualObject(ROOT, this, QDBusConnection::SubPath);
    bus.registerService(SERVICE);
}

FakeSecretService::~FakeSecretService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterService(SERVICE);
    bus.unregisterObject(ROOT, QDBusConnection::UnregisterTree);
}

bool FakeSecretService::busAvailable()
{
    return s_bus.available() && QDBusConnection::sessionBus().isConnected();
}

QByteArray FakeSecretService::secretFor(const QString& key) const
{
    const QList<QString> found = search({{"application", "eurooffice-desktopeditors"}, {"key", key}});
    return found.isEmpty() ? QByteArray() : items[found.first()].secret;
}

void FakeSecretService::addItem(const QString& key, const QByteArray& secret, bool locked)
{
    Item item;
    item.attributes = {{"application", "eurooffice-desktopeditors"}, {"key", key}};
    item.secret = secret;
    item.locked = locked;
    items[COLLECTION + "/" + QString::number(m_nextId++)] = item;
}

QList<QString> FakeSecretService::search(const QMap<QString, QString>& attributes) const
{
    QList<QString> out;
    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        bool match = true;
        for (auto a = attributes.cbegin(); a != attributes.cend(); ++a)
            match = match && it->attributes.value(a.key()) == a.value();
        if (match)
            out.append(it.key());
    }
    return out;
}

QString FakeSecretService::newPrompt(bool unlock, const QList<QString>& objects)
{
    const QString path = ROOT + "/prompt/" + QString::number(m_nextId++);
    m_prompts[path] = unlock ? objects : QList<QString>{};
    return path;
}

QString FakeSecretService::introspect(const QString& path) const
{
    return QStringLiteral("<interface name=\"%1\"/>").arg(interfaceOf(path));
}

bool FakeSecretService::handleMessage(const QDBusMessage& message, const QDBusConnection& connection)
{
    const QString path = message.path();
    const QString member = message.member();
    const QList<QVariant> args = message.arguments();
    QDBusConnection bus(connection);

    if (failingMethods.contains(member)) {
        bus.send(message.createErrorReply(QDBusError::Failed, "failing on purpose"));
        return true;
    }

    if (member == "OpenSession") {
        bus.send(message.createReply({QVariant::fromValue(QDBusVariant(QString())),
                                      QVariant::fromValue(QDBusObjectPath(ROOT + "/session/1"))}));
    } else if (member == "ReadAlias") {
        bus.send(message.createReply(QVariant::fromValue(QDBusObjectPath(hasDefaultCollection ? COLLECTION : NO_PROMPT))));
    } else if (member == "SearchItems") {
        QList<QString> unlocked, locked;
        for (const QString& item : search(qdbus_cast<FakeStringMap>(args.value(0))))
            (items[item].locked ? locked : unlocked).append(item);
        bus.send(message.createReply({pathList(unlocked), pathList(locked)}));
    } else if (member == "Unlock") {
        QList<QString> unlocked, locked;
        for (const QDBusObjectPath& p : qdbus_cast<QList<QDBusObjectPath>>(args.value(0))) {
            const bool isLocked = p.path() == COLLECTION ? collectionLocked : items.value(p.path()).locked;
            (isLocked ? locked : unlocked).append(p.path());
        }
        bus.send(message.createReply({pathList(unlocked),
                QVariant::fromValue(QDBusObjectPath(locked.isEmpty() ? NO_PROMPT : newPrompt(true, locked)))}));
    } else if (member == "GetSecrets") {
        FakeSecretMap secrets;
        for (const QDBusObjectPath& p : qdbus_cast<QList<QDBusObjectPath>>(args.value(0))) {
            if (items.contains(p.path()) && !items[p.path()].locked)
                secrets[p] = FakeSecret{qdbus_cast<QDBusObjectPath>(args.value(1)), {}, items[p.path()].secret, "text/plain"};
        }
        bus.send(message.createReply(QVariant::fromValue(secrets)));
    } else if (member == "CreateItem") {
        const QVariantMap props = qdbus_cast<QVariantMap>(args.value(0));
        Item item;
        item.label = props.value("org.freedesktop.Secret.Item.Label").toString();
        item.attributes = qdbus_cast<FakeStringMap>(props.value("org.freedesktop.Secret.Item.Attributes"));
        item.secret = qdbus_cast<FakeSecret>(args.value(1)).value;

        // replace an item with the same attributes, as asked by the app
        const QList<QString> existing = search(item.attributes);
        const QString itemPath = existing.isEmpty() ? COLLECTION + "/" + QString::number(m_nextId++) : existing.first();
        QString prompt = NO_PROMPT;
        if (promptOnCreate) {
            prompt = newPrompt(false, {});
            m_pending[prompt] = [this, itemPath, item] { items[itemPath] = item; };
        } else items[itemPath] = item;
        bus.send(message.createReply({QVariant::fromValue(QDBusObjectPath(promptOnCreate ? "/" : itemPath)),
                                      QVariant::fromValue(QDBusObjectPath(prompt))}));
    } else if (member == "Delete") {
        QString prompt = NO_PROMPT;
        if (promptOnDelete) {
            prompt = newPrompt(false, {});
            m_pending[prompt] = [this, path] { items.remove(path); };
        } else items.remove(path);
        bus.send(message.createReply(QVariant::fromValue(QDBusObjectPath(prompt))));
    } else if (member == "Prompt" && m_prompts.contains(path)) {
        ++promptCount;
        const QList<QString> objects = m_prompts.take(path);
        const auto action = m_pending.take(path);
        if (!dismissPrompts) {
            for (const QString& p : objects) {
                if (p == COLLECTION)
                    collectionLocked = false;
                else if (items.contains(p))
                    items[p].locked = false;
            }
            if (action)
                action();
        }
        bus.send(message.createReply());

        QDBusMessage completed = QDBusMessage::createSignal(path, "org.freedesktop.Secret.Prompt", "Completed");
        completed << dismissPrompts << QVariant::fromValue(QDBusVariant(QString()));
        bus.send(completed);
    } else {
        return false;
    }
    return true;
}

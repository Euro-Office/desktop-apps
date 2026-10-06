#ifndef GATEWAY_GATEWAYSERVER_H
#define GATEWAY_GATEWAYSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QString>

class CAscApplicationManager;

namespace Gateway
{
    class GatewayCommandRunner;

    // Authenticated external listener. Terminates the unix-socket transport (QLocalServer
    // on Linux is a real AF_UNIX SOCK_STREAM socket — see cdp-gateway-cli-plan.md §0 for
    // why this is NOT built on desktop-apps' existing singleapplication.cpp/CSocket UDP
    // mechanism). tcp-loopback transport is a documented future addition, not implemented
    // in this pass (--transport flag currently only accepts "unix-socket").
    class GatewayServer : public QObject
    {
        Q_OBJECT
    public:
        explicit GatewayServer(CAscApplicationManager* manager, QObject* parent = nullptr);

        // Starts listening on $XDG_RUNTIME_DIR/eo-gateway-<uid>.sock (mode 0600) and
        // writes a fresh random token to $XDG_RUNTIME_DIR/eo-gateway-<uid>.token
        // (mode 0600) per plan §0. Returns false if either the socket or the token
        // file could not be created (e.g. a stale gateway from a crashed previous
        // instance still holds the socket path).
        bool Start();
        void Stop();

    private slots:
        void OnNewConnection();

    private:
        static QString SocketPath();
        static QString TokenPath();
        QString GenerateAndPersistToken();

        QLocalServer m_server;
        QString m_token;
        GatewayCommandRunner* m_runner;
    };
}

#endif // GATEWAY_GATEWAYSERVER_H

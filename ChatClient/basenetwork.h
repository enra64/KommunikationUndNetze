#ifndef BASENETWORK_H
#define BASENETWORK_H

#include <QObject>

#include <QString>
#include <QtConcurrent/QtConcurrent>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <poll.h>

#include "message.h"

enum struct ConnectionState{
    SERVER,
    CLIENT,
    NO_CONNECTION,
    NOT_SET
};

enum struct NetworkError{
    ERROR_NO_ERROR,
    PORT_NO_INTEGER,
    AWAIT_CONTACT_FAILED,
    BIND_FAILED,
    LISTEN_FAILED,
    ACCEPT_FAILED,
    SOCKET_FAILED,
    CONTACT_FAILED,
    HOST_NOT_RESOLVED,
    MAKE_CONTACT_FAILED
};

class BaseNetwork : public QObject
{
    Q_OBJECT
public:
    explicit BaseNetwork(QObject *parent = 0);
    int closeNetwork();
    int send(const QString msg);
    virtual NetworkError server(const QString port);
    virtual NetworkError client(const QString host, const QString port);
    ConnectionState getConnectionState();
signals:
    void clientConnected(NetworkError error, Peer& newClient);
    void disconnect(Peer name, int remainingNetworkConnections);
    void messageReceived(Message msg);
    void closed(int status);
protected slots:
    void onPoll();
protected:
    ConnectionState mConnectionState = ConnectionState::NOT_SET;
    QString networkToString(int fd, size_t *receiveLength);
    bool parsePort(const QString port, short& shortPort);
    QFutureWatcher<int> mServerWaitWatcher;
    std::vector<Peer>* mClients;
    int mZeroLengthMsgCount = 0;
    int mServerSocketHandle;
    char mBuffer[1024];
    short mPort;
    long mHost;
    void checkForNewClients(struct pollfd structs[], int clientCount);
private:
    QTimer* mTimer;
};

#endif // BASENETWORK_H

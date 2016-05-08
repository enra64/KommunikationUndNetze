#ifndef BASENETWORK_H
#define BASENETWORK_H

#include <QObject>

#include <QString>
#include <QtConcurrent/QtConcurrent>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "message.h"

enum struct ConnectionState{
    SERVER,
    CLIENT,
    NO_CONNECTION,
    NOT_SET
};

class BaseNetwork : public QObject
{
    Q_OBJECT
public:
    explicit BaseNetwork(QObject *parent = 0);
    virtual int closeNetwork();
    virtual int send(const QString msg);
    virtual int server(const QString port);
    virtual int client(const QString host, const QString port);
signals:
    void clientConnected(bool success);
    void disconnect(QString name, int remainingNetworkConnections);
    void messageReceived(Message msg);
protected slots:
    virtual int onPoll();
    virtual void onAccept();
    virtual int waitForClients(struct sockaddr* clientStruct);
protected:
    ConnectionState mConnectionState = ConnectionState::NOT_SET;
    QString networkToString(int fd, size_t& receiveLength);
    bool parsePort(const QString port, short& shortPort);
    QFutureWatcher<int> mServerWaitWatcher;
    int mZeroLengthMsgCount = 0;
    char mBuffer[1024];
private:
    QTimer* mTimer;
};

#endif // BASENETWORK_H

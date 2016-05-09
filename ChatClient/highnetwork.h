#ifndef HIGHNETWORK_H
#define HIGHNETWORK_H

#include <QtConcurrent/QtConcurrent>
#include <QString>
#include <QObject>

#include "cnaiapi.h"

#include <netinet/in.h>
#include <poll.h>

#include <stdio.h>
#include <string.h>

#include "basenetwork.h"

class HighNetwork : public BaseNetwork
{
    Q_OBJECT
public:
    HighNetwork(QObject *parent = 0);
    int send(const QString msg) override;
    int closeNetwork() override;
    int server(const QString port) override;
    int client(const QString host, const QString port) override;
protected slots:
    int onPoll() override;
private:
    int asyncWaitForClients(struct sockaddr* clientStruct);
    int mZeroLengthMsgCount = 0;
    connection waitAsServer();
    connection mServerSocketHandle;
    void onAccept();
    computer mHost;
    appnum mPort;
};

#endif // HIGHNETWORK_H

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
    size_t receive(std::vector<Message>& msg) override;
    int closeNetwork() override;
    int server(const QString port) override;
    int client(const QString host, const QString port) override;
private:
    QFutureWatcher<connection> mServerWaitWatcher;
    int mZeroLengthMsgCount = 0;
    connection waitAsServer();
    connection mNetwork;
    computer mHost;
    appnum mPort;
public slots:
    void handleServerWaitFinished();
};

#endif // HIGHNETWORK_H

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
    NetworkError server(const QString port) override;
    NetworkError client(const QString host, const QString port) override;
protected slots:
    void onAccept();
private:
    int asyncWaitForClients(struct sockaddr* clientStruct);
    connection waitAsServer();
};

#endif // HIGHNETWORK_H

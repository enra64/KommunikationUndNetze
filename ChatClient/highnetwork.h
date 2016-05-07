#ifndef HIGHNETWORK_H
#define HIGHNETWORK_H

#include <QObject>
#include "cnaiapi.h"
#include "chatwindow.h"
#include <netinet/in.h>
#include <poll.h>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <stdio.h>
#include <string.h>
#include "networkbase.h"

class HighNetwork: public NetworkBase
{
    Q_OBJECT
public:
    HighNetwork(ChatWindow* cw);
    int send(const QString msg) override;
    size_t receive(std::vector<QString>& msg) override;
    int closeNetwork() override;
    int server(const QString port) override;
    int client(const QString host, const QString port) override;
private:
    QFutureWatcher<connection> mServerWaitWatcher;
    bool parsePort(const QString port);
    int mZeroLengthMsgCount = 0;
    connection waitAsServer();
    ChatWindow* mChatWindow;
    connection mNetwork;
    computer mHost;
    appnum mPort;
};

#endif // HIGHNETWORK_H

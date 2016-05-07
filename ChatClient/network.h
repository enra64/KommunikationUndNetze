#ifndef NETWORK_H
#define NETWORK_H


class ChatWindow;
#include "chatwindow.h"
#include "cnaiapi.h"
#include <netinet/in.h>
#include <poll.h>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <stdio.h>
#include <string.h>

class Network : public QObject
{
        Q_OBJECT
public:
    Network(ChatWindow *cw);
    void send(const QString msg);
    void closeNetwork();
    void server(const QString port);
    void client(const QString host, const QString port);
private:
    QFutureWatcher<connection> mServerWaitWatcher;
    connection waitAsServer();
    size_t networkToString(QString& msg);
    appnum mPort;
    computer mHost;
    connection mNetwork;
    char mBuffer[1024];
    ChatWindow* mChatWindow;
    int mZeroLengthMsgCount = 0;
public slots:
    void pollingRead();
    void handleServerWaitFinished();
};

#endif // NETWORK_H

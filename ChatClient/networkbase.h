#ifndef NETWORKBASE_H
#define NETWORKBASE_H

#include "chatwindow.h"
#include "cnaiapi.h"
#include <netinet/in.h>
#include <poll.h>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <stdio.h>
#include <string.h>

class NetworkBase
{
public:
    NetworkBase(ChatWindow& cw);
    virtual int closeNetwork();
    virtual size_t receive(std::vector<QString>& msg);
    virtual int send(const QString msg);
    virtual int server(const QString port);
    virtual int client(const QString host, const QString port);
protected:
    QString networkToString(int fd, size_t& receiveLength);
    bool parsePort(const QString port);
    int mZeroLengthMsgCount = 0;
    char mBuffer[1024];
private:
    ChatWindow* mChatWindow;
};

#endif // NETWORKBASE_H

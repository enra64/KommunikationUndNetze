#ifndef HIGHNETWORK_H
#define HIGHNETWORK_H

#include "networkbase.h"

class HighNetwork : public NetworkBase
{
public:
    HighNetwork();
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

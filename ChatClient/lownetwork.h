#ifndef LOWNETWORK_H
#define LOWNETWORK_H

#include <QObject>
#include <QString>

#include <arpa/inet.h>
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>

#include <stdio.h>

#include "compilerdistractor.h"

#include "basenetwork.h"

class LowNetwork : public BaseNetwork
{
        Q_OBJECT
public:
    LowNetwork(QObject *parent = 0);
    ~LowNetwork();
    int closeNetwork() override;
    int send(const QString msg) override;
    size_t receive(std::vector<QString>& msg) override;
    int server(const QString port) override;
    int client(const QString host, const QString port) override;
private:
    bool mIsServer;
    unsigned long mAdress;
    short mPort;
    int mServerSocketHandle;
    std::vector<int>* mClientSocketHandles;
    bool getHostAddress(const QString hostName, unsigned long& hostInt);
};

#endif // LOWNETWORK_H

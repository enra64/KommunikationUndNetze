#ifndef LOWNETWORK_H
#define LOWNETWORK_H

#include <QString>
#include <arpa/inet.h>
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>

class LowNetwork
{
public:
    int server(const QString port);
    int client(const QString host, const QString port);
private:
    unsigned long mAdress;
    short mPort;
    int mSocketHandle;
    bool getHostAddress(const QString hostName, unsigned long& hostInt);
};

#endif // LOWNETWORK_H

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
    bool getHostAddress(QString hostName, unsigned long& hostInt);
    bool getSocket(int& socketHandle, unsigned long address, int port);
};

#endif // LOWNETWORK_H

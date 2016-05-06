#include "lownetwork.h"

int LowNetwork::server(const QString port){
    bool validPort;
    short shortport = port.toShort(&validPort);
    if(!validPort)
        return -1;

    mAdress = htonl(INADDR_ANY);

    struct sockaddr_in serverStruct, clientStruct;
    mSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mSocketHandle < 0)
        return -3;

    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(shortport);
    serverStruct.sin_addr.s_addr = mAdress;

    if(bind(mSocketHandle, (struct sockaddr *) &serverStruct, sizeof(serverStruct)) < 0)
        return -4;

    if(listen(mSocketHandle, 4) < 0)
        return -5;

    int clientSocketHandle;

    unsigned int clientLength = sizeof(clientStruct);
    if((clientSocketHandle = accept(mSocketHandle, (struct sockaddr *) &clientStruct, &clientLength)) < 0)
        return -6;

    // successfully navigated the mine field
    return 0;
}

int LowNetwork::client(const QString host, const QString port){
    bool validPort;
    short shortport = port.toShort(&validPort);

    if(!validPort)
        return -1;

    if(!getHostAddress(host, mAdress))
        return -2;

    struct sockaddr_in peerDescription;
    mSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mSocketHandle < 0)
        return -3;

    peerDescription.sin_family = AF_INET;
    peerDescription.sin_port = htons(shortport);
    peerDescription.sin_addr.s_addr = mAdress;

    if(connect(mSocketHandle, (struct sockaddr *) &peerDescription, sizeof(peerDescription)) < 0)
        return -4;

    return 0;
}

bool LowNetwork::getHostAddress(const QString hostName, unsigned long& address){
    struct hostent * host;
    struct in_addr h_addr;

    host = gethostbyname(hostName.toStdString().c_str());

    if(host == NULL)
        return false;

    h_addr.s_addr = *((unsigned long *) host -> h_addr_list[0]);
    address = *((unsigned long *) host -> h_addr_list[0]);
    return true;
}

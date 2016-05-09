#include "lownetwork.h"

LowNetwork::LowNetwork(QObject *parent) : BaseNetwork(parent){
    mConnectionState = ConnectionState::NO_CONNECTION;
}

LowNetwork::~LowNetwork(){
    mServerWaitWatcher.cancel();
    delete mClients;
}

int LowNetwork::server(const QString port){
    bool validPort;
    short shortport = port.toShort(&validPort);
    if(!validPort)
        return -1;

    mHost = htonl(INADDR_ANY);

    struct sockaddr_in serverStruct;
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(mServerSocketHandle < 0)
        return -2;

    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(shortport);
    serverStruct.sin_addr.s_addr = mHost;

    if(bind(mServerSocketHandle, (struct sockaddr *) &serverStruct, sizeof(serverStruct)) < 0)
        return -3;

    if(listen(mServerSocketHandle, 4) < 0)
        return -4;

    mConnectionState = ConnectionState::SERVER;

    // successfully navigated the mine field
    return 0;
}

int LowNetwork::client(const QString host, const QString port){
    bool validPort;
    short shortport = port.toShort(&validPort);

    if(!validPort)
        return -1;

    if(!getHostAddress(host, mHost))
        return -2;

    struct sockaddr_in peerDescription;
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mServerSocketHandle < 0)
        return -3;

    peerDescription.sin_family = AF_INET;
    peerDescription.sin_port = htons(shortport);
    peerDescription.sin_addr.s_addr = mHost;

    if(Connector::myConnect(mServerSocketHandle, (struct sockaddr *) &peerDescription, sizeof(peerDescription)) < 0)
        return -4;

    mConnectionState = ConnectionState::CLIENT;

    return 0;
}

bool LowNetwork::getHostAddress(const QString hostName, long& address){
    struct hostent * host;
    struct in_addr h_addr;

    host = gethostbyname(hostName.toStdString().c_str());

    if(host == NULL)
        return false;

    h_addr.s_addr = *((unsigned long *) host -> h_addr_list[0]);
    address = *((long *) host -> h_addr_list[0]);
    return true;
}

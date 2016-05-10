#include "lownetwork.h"

LowNetwork::LowNetwork(QObject *parent) : BaseNetwork(parent){
    mConnectionState = ConnectionState::NO_CONNECTION;
}

LowNetwork::~LowNetwork(){
    mServerWaitWatcher.cancel();
    delete mClients;
}

NetworkError LowNetwork::server(const QString port){
    bool validPort;
    short shortport = port.toShort(&validPort);
    if(!validPort)
        return NetworkError::PORT_NO_INTEGER;

    mHost = htonl(INADDR_ANY);

    struct sockaddr_in serverStruct;
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(mServerSocketHandle < 0)
        return NetworkError::SOCKET_FAILED;

    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(shortport);
    serverStruct.sin_addr.s_addr = mHost;

    if(bind(mServerSocketHandle, (struct sockaddr *) &serverStruct, sizeof(serverStruct)) < 0)
        return NetworkError::BIND_FAILED;

    if(listen(mServerSocketHandle, 4) < 0)
        return NetworkError::LISTEN_FAILED;

    mConnectionState = ConnectionState::SERVER;

    // successfully navigated the mine field
    return NetworkError::ERROR_NO_ERROR;
}

NetworkError LowNetwork::client(const QString host, const QString port){
    bool validPort;
    short shortport = port.toShort(&validPort);

    if(!validPort)
        return NetworkError::PORT_NO_INTEGER;

    if(!getHostAddress(host, mHost))
        return NetworkError::HOST_NOT_RESOLVED;

    struct sockaddr_in peerDescription;
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mServerSocketHandle < 0)
        return NetworkError::SOCKET_FAILED;

    peerDescription.sin_family = AF_INET;
    peerDescription.sin_port = htons(shortport);
    peerDescription.sin_addr.s_addr = mHost;

    if(Connector::myConnect(mServerSocketHandle, (struct sockaddr *) &peerDescription, sizeof(peerDescription)) < 0)
        return NetworkError::CONTACT_FAILED;

    mConnectionState = ConnectionState::CLIENT;

    return NetworkError::ERROR_NO_ERROR;
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

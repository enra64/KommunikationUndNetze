#include "lownetwork.h"

LowNetwork::LowNetwork(ChatWindow *cw) : NetworkBase(cw){
    mClientSocketHandles = new std::vector<int>();
}

LowNetwork::~LowNetwork(){
    delete mClientSocketHandles;
}

size_t LowNetwork::receive(std::vector<QString>& msg){
    if(mIsServer){
        struct pollfd structs[mClientSocketHandles->size()];
        for(size_t i = 0; i < mClientSocketHandles->size(); i++){
            structs[i].fd = mClientSocketHandles->at(i);
            structs[i].events = POLLIN;
            structs[i].revents = 0;
        }
        if(poll(structs, mClientSocketHandles->size(), 2) < 0){
            closeNetwork();
            return -1;
        }
        msg.resize(mClientSocketHandles->size());
        int receiveCount = 0;
        for(size_t i = 0; i < mClientSocketHandles->size(); i++){
            if(structs[i].revents & POLLIN){
                receiveCount++;
                size_t readLength;
                msg[i] = networkToString(structs[i].fd, readLength);
            }
        }
        return receiveCount;
    }
    //isClient
    else{
        struct pollfd pollingStruct[1] = {{mServerSocketHandle, POLLIN, 0}};
        if(poll(pollingStruct, 1, 2) < 0){
            closeNetwork();
            return -1;
        }
        if(pollingStruct[0].revents & POLLIN){
            size_t readLength;
            msg.push_back(networkToString(pollingStruct[0].fd, readLength));
            return 1;
        }
        return 0;
    }
}

int LowNetwork::send(const QString msg){
    char* msgData = msg.toLatin1().data();
    int sentData;
    if(mIsServer){
        for(int fd : *mClientSocketHandles){
            sentData = write(fd, msgData, msg.length());
            if(sentData != msg.length())
                return -1;
        }
    }
    else{
        sentData = write(mServerSocketHandle, msgData, msg.length());
        if(sentData != msg.length())
            return -1;
    }
    return 0;
}

int LowNetwork::closeNetwork(){
    int errorCount = 0;
    if(mIsServer){
        for(int fd : *mClientSocketHandles)
            if(close(fd) < 0)
                errorCount--;
    }
    else{
        if(close(mServerSocketHandle) < 0)
            errorCount--;
    }

    mAdress = -1;
    mPort = -1;
    mServerSocketHandle = -1;
    mClientSocketHandles->clear();

    return errorCount;
}

int LowNetwork::server(const QString port){
    bool validPort;
    short shortport = port.toShort(&validPort);
    if(!validPort)
        return -1;

    mAdress = htonl(INADDR_ANY);

    struct sockaddr_in serverStruct, clientStruct;
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mServerSocketHandle < 0)
        return -3;

    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(shortport);
    serverStruct.sin_addr.s_addr = mAdress;

    if(bind(mServerSocketHandle, (struct sockaddr *) &serverStruct, sizeof(serverStruct)) < 0)
        return -4;

    if(listen(mServerSocketHandle, 4) < 0)
        return -5;

    int clientSocketHandle;

    unsigned int clientLength = sizeof(clientStruct);
    if((clientSocketHandle = accept(mServerSocketHandle, (struct sockaddr *) &clientStruct, &clientLength)) < 0)
        return -6;

    mIsServer = true;

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
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mServerSocketHandle < 0)
        return -3;

    peerDescription.sin_family = AF_INET;
    peerDescription.sin_port = htons(shortport);
    peerDescription.sin_addr.s_addr = mAdress;

    if(Connector::myConnect(mServerSocketHandle, (struct sockaddr *) &peerDescription, sizeof(peerDescription)) < 0)
        return -4;

    mIsServer = false;

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

#include "lownetwork.h"

LowNetwork::LowNetwork(){
    mClientSocketHandles = new std::vector<int>();
}

LowNetwork::~LowNetwork(){
    delete mClientSocketHandles;
}

size_t LowNetwork::receive(std::vector<QString>& msg){
    if(isServer){
        struct pollfd structs[mClientSocketHandles->size()];
        for(int i = 0; i < mClientSocketHandles->size(); i++)
            structs[i] = {mClientSocketHandles[i], POLLIN, 0};
        if(poll(structs, mClientSocketHandles->size(), 2) < 0){
            closeNetwork();
            return -1;
        }
        msg.resize(mClientSocketHandles->size());
        int receiveCount = 0;
        for(int i = 0; i < mClientSocketHandles->size(); i++){
            if(pollStructs[i].revents & POLLIN){
                receiveCount++;
                size_t readLength;
                msg[i] = networkToString(pollStructs[i].fd, readLength);
            }
        }
        return receiveCount;
    }
    //isClient
    else{
        struct pollfd pollingStruct[1];
        pollingStruct[0] = {{mServerSocketHandle, POLLIN, 0}};
        if(poll(pollingStruct, 1, 2) < 0){
            closeNetwork();
            return -1;
        }
        if(pollStructs[0].revents & POLLIN){
            size_t readLength;
            msg[i] = networkToString(pollStructs[0].fd, readLength);
            return 1;
        }
        return 0;
    }
}

int LowNetwork::send(const QString msg){
    char* msgData = msg.toLatin1().data();
    size_t sentData;
    if(mIsServer){
        for(int fd : mClientSocketHandles){
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
    for(int fd : mClientSocketHandles)
        close(fd);
    close(mServerSocketHandle);

    mAdress = -1;
    mPort = -1;
    mServerSocketHandle = -1;
    mClientSocketHandles->clear();
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

    if(connect(mServerSocketHandle, (struct sockaddr *) &peerDescription, sizeof(peerDescription)) < 0)
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

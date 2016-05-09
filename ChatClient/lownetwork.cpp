#include "lownetwork.h"

LowNetwork::LowNetwork(QObject *parent) : BaseNetwork(parent){
    mConnectionState = ConnectionState::NO_CONNECTION;
}

LowNetwork::~LowNetwork(){
    mServerWaitWatcher.cancel();
    delete mClients;
}

int LowNetwork::send(const QString msg){
    char* msgData = msg.toLatin1().data();
    int sentData;
    switch(mConnectionState){
        case ConnectionState::CLIENT:
        sentData = write(mServerSocketHandle, msgData, msg.length());
        if(sentData != msg.length())
            return -1;
        return 0;
    case ConnectionState::SERVER:
        for(Peer p : *mClients){
            sentData = write(p.getSocket(), msgData, msg.length());
            if(sentData != msg.length())
                return -1;
        }
        return 0;
    case ConnectionState::NOT_SET:
    case ConnectionState::NO_CONNECTION:
        return -2;
    }
    return -3;
}

int LowNetwork::closeNetwork(){
    int errorCount = 0;
    for(Peer p : *mClients)
        if(close(p.getSocket()) < 0)
            errorCount--;
    if(close(mServerSocketHandle) < 0)
        errorCount--;


    mServerWaitWatcher.cancel();
    // ensure that no accept() is currently waiting
    mServerWaitWatcher.waitForFinished();


    mAdress = -1;
    mPort = -1;
    mServerSocketHandle = -1;
    mClients->clear();

    mConnectionState = ConnectionState::NO_CONNECTION;

    emit closed(errorCount);
    return errorCount;
}

bool peerHasSocket (Peer p, int socket) {
  return p.getSocket() == socket;
}

int LowNetwork::onPoll(){
    switch(mConnectionState){
        case ConnectionState::NOT_SET:
        case ConnectionState::NO_CONNECTION:
            return -1337;
        case ConnectionState::SERVER:{
                size_t clientCount = mClients->size();
                struct pollfd structs[clientCount + 1];
                int receiveCount = 0;

                for(size_t i = 0; i < clientCount; i++){
                    structs[i].fd = mClients->at(i).getSocket();
                    structs[i].events = POLLIN;
                    structs[i].revents = 0;
                }

                // check whether we have any new connections
                structs[clientCount].fd = mServerSocketHandle;
                structs[clientCount].events = POLLIN;
                structs[clientCount].revents = 0;

                if(poll(structs, clientCount + 1, 2) < 0){
                    emit disconnect("Everything", 0);
                    closeNetwork();
                    return -1;
                }

                for(size_t i = 0; i < clientCount; i++){
                    if(structs[i].revents & POLLIN){
                        receiveCount++;
                        size_t readLength;
                        Message m(networkToString(structs[i].fd, readLength), mClients->at(i));
                        if(m.isEmpty()){
                            close(structs[i].fd);
                            emit disconnect(mClients->at(i).getName(), mClients->size() - 1);
                            mClients->erase(mClients->begin() + i);
                        }
                        else
                            emit messageReceived(m);
                    }
                }

                // check for new clients now that we cant fuck up the vector anymore
                if(structs[clientCount].revents & POLLIN){
                    struct sockaddr_in clientStruct;
                    unsigned int clientLength = sizeof(clientStruct);
                    int clientSocket = accept(mServerSocketHandle, (struct sockaddr *) &clientStruct, &clientLength);
                    emit clientConnected(clientSocket >= 0);
                    if(clientSocket >= 0)
                        mClients->push_back(Peer(clientSocket));
                }

                return receiveCount;
        }
        case ConnectionState::CLIENT:{
                struct pollfd pollingStruct[1] = {{mServerSocketHandle, POLLIN, 0}};
                if(poll(pollingStruct, 1, 2) < 0){
                    emit disconnect("The server", 0);
                    closeNetwork();
                    return -1;
                }
                if(pollingStruct[0].revents & POLLIN){
                    size_t readLength;
                    Message m(networkToString(pollingStruct[0].fd, readLength), Peer("Server", mServerSocketHandle));
                    if(readLength > 0)
                        emit messageReceived(m);
                    else{
                        emit disconnect("The server", 0);
                        closeNetwork();
                        return -1;
                    }
                    return 1;
                }
                return 0;
        }
    }
    return -18;
}

int LowNetwork::server(const QString port){
    bool validPort;
    short shortport = port.toShort(&validPort);
    if(!validPort)
        return -1;

    mAdress = htonl(INADDR_ANY);

    struct sockaddr_in serverStruct;
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(mServerSocketHandle < 0)
        return -2;

    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(shortport);
    serverStruct.sin_addr.s_addr = mAdress;

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

    mConnectionState = ConnectionState::CLIENT;

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

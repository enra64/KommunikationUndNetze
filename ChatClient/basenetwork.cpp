#include "basenetwork.h"

BaseNetwork::BaseNetwork(QObject *parent) : QObject(parent)
{
    mClients = new std::vector<Peer>();
    mTimer = new QTimer(this);
    QObject::connect(
                mTimer,
                SIGNAL(timeout()),
                this,
                SLOT(onPoll()));
    mTimer->start(4);
}

int BaseNetwork::closeNetwork(){
    int errorCount = 0;

    for(Peer p : *mClients)
        if(close(p.getSocket()) < 0)
            errorCount--;

    if(close(mServerSocketHandle) < 0)
        errorCount--;


    mServerWaitWatcher.cancel();
    // ensure that no accept() is currently waiting
    mServerWaitWatcher.waitForFinished();

    // only send when a connection existed/tried
    if(mConnectionState != ConnectionState::NO_CONNECTION)
        emit closed(0);

    mHost = -1;
    mPort = -1;
    mServerSocketHandle = -1;
    mClients->clear();

    mConnectionState = ConnectionState::NO_CONNECTION;

    return errorCount;
}

bool BaseNetwork::parsePort(const QString port, short& shortPort){
    bool portIsInt;
    shortPort = port.toInt(&portIsInt);
    return portIsInt;
}

QString BaseNetwork::networkToString(int fd, size_t& rLength){
    rLength = read(fd, mBuffer, sizeof(mBuffer));
    mBuffer[rLength] = '\0';
    return QString(mBuffer);
}

int BaseNetwork::onPoll(){
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

NetworkError BaseNetwork::client(const QString, const QString){
    return (NetworkError) -1;
}

ConnectionState BaseNetwork::getConnectionState()
{
    return mConnectionState;
}

NetworkError BaseNetwork::server(const QString){
    return (NetworkError) -1;
}

int BaseNetwork::send(const QString msg){
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

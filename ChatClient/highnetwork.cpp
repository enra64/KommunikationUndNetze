#include "highnetwork.h"

HighNetwork::HighNetwork(QObject *parent) : BaseNetwork(parent){
    mConnectionState = ConnectionState::NO_CONNECTION;

    QObject::connect(
                &mServerWaitWatcher,
                SIGNAL(finished()),
                this,
                SLOT(onAccept()));
}

int HighNetwork::closeNetwork() {
    int ret = end_contact(mServerSocketHandle);

    emit disconnect("Network", 0);

    // reset member variables
    mZeroLengthMsgCount = 0;
    mServerSocketHandle = -1;
    mPort = -1;
    mHost = -1;

    mConnectionState = ConnectionState::NO_CONNECTION;

    return ret;
}

int HighNetwork::send(const QString msg) {
    // write to output
//    int written = write(mNetwork, msg.toLatin1().data(), msg.length());
//    if(written != msg.length())
//        return -1;
//    return 0;
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

int HighNetwork::onPoll(){
    // polling structs
//    struct pollfd pollStructs[1] = {{mNetwork, POLLIN, 0}}; // poll network
//    if(poll(pollStructs, 1, 2) < 0){
//        if(closeNetwork() < 0)
//            return -2;
//        return -1;
//    }
//    if(pollStructs[0].revents & POLLIN){
//        size_t readLength;
//        QString mess = networkToString(pollStructs[0].fd, readLength);
//        if(readLength != 0)
//            emit messageReceived(Message(mess, "Not you: "));
//        else{
//            if(closeNetwork() < 0)
//                return -4;
//            return -3;
//        }
//    }
//    return 0;
    switch(mConnectionState){
        case ConnectionState::NOT_SET:
        case ConnectionState::NO_CONNECTION:
            return -1337;
        case ConnectionState::SERVER:{
                size_t clientCount = mClients->size();
                struct pollfd structs[clientCount + 1];
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
                int receiveCount = 0;

                if(structs[clientCount].revents & POLLIN){
                    struct sockaddr_in clientStruct;
                    unsigned int clientLength = sizeof(clientStruct);
                    int clientSocket = accept(mServerSocketHandle, (struct sockaddr *) &clientStruct, &clientLength);
                    emit clientConnected(clientSocket >= 0);
                    if(clientSocket >= 0)
                        mClients->push_back(Peer(clientSocket));
                }

                for(size_t i = 0; i < mClients->size(); i++){
                    if(structs[i].revents & POLLIN){
                        receiveCount++;
                        size_t readLength;
                        Message m(networkToString(structs[i].fd, readLength), mClients->at(i));
                        if(m.isEmpty()){
                            close(structs[i].fd);
                            mClients->erase(mClients->begin() + i);
                            emit disconnect(mClients->at(i).getName(), mClients->size());
                        }
                        else
                            emit messageReceived(m);
                    }
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
                    emit messageReceived(Message(networkToString(pollingStruct[0].fd, readLength), Peer("Server", mServerSocketHandle)));
                    return 1;
                }
                return 0;
        }
    }
    return -18;
}

int HighNetwork::asyncWaitForClients(struct sockaddr*){
    return await_contact(mPort);
}

void HighNetwork::onAccept(){
    mClients->push_back(Peer(mServerWaitWatcher.result()));
    mServerSocketHandle = get_server_socket(mPort);
    clientConnected(mClients->size());
}

int HighNetwork::server(const QString port) {
    if(!parsePort(port, mPort))
        return -1;

    mServerWaitWatcher.cancel();

    mConnectionState = ConnectionState::SERVER;

    // start connection in concurrent thread
    QFuture<connection> future = QtConcurrent::run(this, &HighNetwork::asyncWaitForClients, nullptr);

    mServerWaitWatcher.setFuture(future);

    return 0;
}

int HighNetwork::client(const QString host, const QString port) {
    if(!parsePort(port, mPort))
        return -1;

    mHost = cname_to_comp(host.toLatin1().data());

    if(mHost == -1)
        return -2;

    mServerSocketHandle = make_contact(mHost, mPort);

    mConnectionState = ConnectionState::CLIENT;

    return mServerSocketHandle;
}

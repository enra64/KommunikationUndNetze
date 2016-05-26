#include "network.h"

/*
 *
 *
 *
 *          NETWORK
 *
 *
 *
 */



Network::Network(QObject *parent) : QObject(parent) {

}

int Network::noSignalSend(int socket, char *data, size_t dataLength)
{
    errno = 0;
    switch(send(socket, data, dataLength, MSG_NOSIGNAL)){
        case -1:
            return -1;
        default:
            if(errno == EPIPE)
                success = -2;
            else
                return 0;
    }
}

QByteArray Network::readNetwork(int fd, size_t *receiveLength) {
    size_t readLength = read(fd, mBuffer, sizeof(mBuffer));
    mBuffer[readLength] = '\0';
    if(rLength != nullptr)
        *rLength = readLength;
    return QByteArray(mBuffer);
}

bool Network::parsePort(const QString &port, short &port) {
    bool portIsInt;
    shortPort = port.toInt(&portIsInt);
    return portIsInt;
}

/*
 *
 *
 *
 *          SERVER
 *
 *
 *
 */
int Server::closeNetwork() {
    int errCount = 0;
    bool networkWasConnected = mServerSocketHandle != 0;

    for(Peer p : *mClients)
        if(close(p.getSocket()) < 0)
            errCount--;

    if(close(mServerSocketHandle) < 0)
        errCount--;

    // emit signal if network was ever connected
    if(networkWasConnected)
        emit networkClosed(errCount);

    delete mClients;
    mServerSocketHandle = -1;
    mPort = -1;

    return errCount;
}

int Server::send(const DataMessage &d) {
    char* msgData = d.getOutData();


    // send to all
    if(d.getTarget().isAllPeer())
        for(size_t i = mClients->size() - 1; i >= 0; i--)
            if(noSignalSend(p.getSocket(), msgData, d.getOutDataLength()) != 0)
                mClients->erase(mClients->begin() + i);

    // only send to one peer
    else
        if(noSignalSend(d.getTarget(), msgData, d.getOutDataLength()) != 0)
            //...
            myList.erase(
                std::remove_if(myList.begin(), myList.end(), [](const Peer& p) { return p.getSocket() == d.getTarget().getSocket(); }), myList.end());

    // free memory
    delete[] msgData;

    return success;
}

void Server::onPoll() {
    // constantly need this shit
    size_t clientCount = mClients->size();

    // check client sockets + server socket for new connections
    struct pollfd structs[clientCount + 1];

    // fill polling struct array
    for(size_t i = 0; i < clientCount + 1; i++) {
        structs[i].fd = i < clientCount ? mClients->at(i).getSocket() : mServerSocketHandle;
        structs[i].events = POLLIN;
        structs[i].revents = 0;
    }

    // run around screaming if we cant poll
    if(poll(structs, clientCount + 1, 2) < 0) {
        // -> Polling error, everything disconnected!
        for(size_t i = 0; i < clientCount; i++)
            emit disconnect(mClients->at(i), 0);
        // close everything else
        closeNetwork();
        return;
    }

    // handle incoming messages
    for(size_t i = 0; i < clientCount; i++) {
        if(structs[i].revents & POLLIN) {
            // create message object on stack
            QByteArray in = networkToByteArray(structs[i].fd, nullptr);

            // if the message is empty, we know a dc happened
            if(in.isEmpty()) {
                // gracefully close socket
                close(structs[i].fd);
                // notify lib user
                emit disconnect(mClients->at(i), mClients->size() - 1);
                // remove peer from list
                mClients->erase(mClients->begin() + i);
            }

            switch((PacketType) in.at(0)) {
            case PacketType::CLIENT_REGISTRATION: {

                break;
            }
            case PacketType::DATA: {
                emit messageReceived(DataMessage(in));
                break;
            }
            default:
                break;
            }
        }
    }

    // check for new clients
    checkForNewClients(structs, clientCount);
    return;
}

void Server::checkForNewClients(pollfd structs[], size_t structLength)
{
    // check for new clients now that we cant fuck up the vector anymore
    if(structs[structLength].revents & POLLIN){
        struct sockaddr_in clientStruct;
        unsigned int clientLength = sizeof(clientStruct);
        int clientSocket = accept(mServerSocketHandle, (struct sockaddr *) &clientStruct, &clientLength);
        if(clientSocket < 0){
            static Peer p;
            clientConnected(NetworkError::ACCEPT_FAILED, p);
        }
        else{
            static Peer p(clientSocket);
            clientConnected(NetworkError::ERROR_NO_ERROR, p);
            mClients->push_back(Peer(p));

        }
    }
}

/*
 *
 *
 *
 *          CLIENT
 *
 *
 *
 */
int Client::closeNetwork() {
    int errCount = 0;
    bool networkWasConnected = mClientSocket != -1;

    // close socket
    errCount += close(mClientSocket);

    // emit signal if network was ever connected
    if(networkWasConnected)
        emit networkClosed(errCount);

    mClientSocket = -1;
    mHost = -1;
    mPort = -1;
    mNetworkId = -1;

    return errCount;
}

int Client::send(const DataMessage &d) {
    // get data
    char* msgData = d.getOutData();

    // ignore all previous errors
    errno = 0;

    switch(send(mClientSocket, msgData, d.getOutDataLength(), MSG_NOSIGNAL)){
        case -1:
            success = -1;
            break;
        case EPIPE:
        default:
            success = 0;
            if(errno == EPIPE)
                success = -2;
    }

    // free memory
    delete[] msgData;

    return success;
}

void Client::onPoll() {
    struct pollfd pollingStruct[1] = {{mServerSocketHandle, POLLIN, 0}};

    // can we poll?
    if(poll(pollingStruct, 1, 2) < 0) {
        emit disconnect(Peer("Server", mServerSocketHandle), 0);
        closeNetwork();
    } else if(pollingStruct[0].revents & POLLIN) {
        QByteArray data = networkToByteArray(pollingStruct[0].fd, nullptr);

        if(data.length() == 0) {
            emit disconnect(Peer("Server", mServerSocketHandle), 0);
            closeNetwork();
        }

        switch((PacketType) data.at(0)) {
        case PacketType::SERVER_REGISTRATION_ANSWER: {
            ServerDataForClientsMessage sdfcm(data);
            clientHandleServerData(sdfcm);
            break;
        }
        case PacketType::DATA:
            emit messageReceived(DataMessage(data));
            break;
        default:
            break;
        }
    }
}

bool Client::getHostAddress(const QString hostName, long& address){
    struct hostent * host;
    struct in_addr h_addr;

    host = gethostbyname(hostName.toStdString().c_str());

    if(host == NULL)
        return false;

    h_addr.s_addr = *((unsigned long *) host -> h_addr_list[0]);
    address = *((long *) host -> h_addr_list[0]);
    return true;
}

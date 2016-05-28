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

void Network::emitClientDiffList(const std::vector<Peer> &oldPeers, const std::vector<Peer> &newPeers) {
    int newElementCount = newPeers.size() - oldPeers.size();
    std::vector<Peer> elementsOnlyInBiggerList(newElementCount);
    const std::vector<Peer> *smallerList = nullptr;
    const std::vector<Peer> *biggerList = nullptr;

    if(newElementCount > 0) {
        smallerList = &oldPeers;
        biggerList = &newPeers;
    } else if(newElementCount < 0) {
        smallerList = &newPeers;
        biggerList = &oldPeers;
    }

    for(size_t biggerListIndex = 0; biggerListIndex < biggerList->size(); biggerListIndex++) {
        bool elementExistsInSmallerList = false;
        int elementSocket = biggerList->at(biggerListIndex).getSocket();

        for(size_t smallerListIndex = 0; smallerListIndex < smallerList->size(); smallerListIndex++) {
            if(elementSocket == smallerList->at(smallerListIndex).getSocket()) {
                elementExistsInSmallerList = true;
                break;
            }
        }

        if(!elementExistsInSmallerList)
            elementsOnlyInBiggerList.push_back(biggerList->at(biggerListIndex));
    }

    emit peerListUpdated(newPeers, elementsOnlyInBiggerList, newElementCount);
}

int Network::noSignalSend(int socket, char* data, size_t dataLength) {
    errno = 0;
    if(-1 == send(socket, data, dataLength, MSG_NOSIGNAL))
        return -1;
    if(errno == EPIPE)
        return -2;
    return 0;
}

QByteArray Network::readNetwork(int fd) {
    size_t readLength = read(fd, mBuffer, sizeof(mBuffer));
    mBuffer[readLength] = '\0';
    return QByteArray(mBuffer);
}

bool Network::parsePort(const QString &portIn, short &portOut) {
    bool portIsInt;
    portOut = portIn.toInt(&portIsInt);
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
Server::Server(const QString& port, NetworkError& result, QObject *parent) : Network(parent)
{
    bool validPort;
    short shortport = port.toShort(&validPort);
    if(!validPort){
        result = NetworkError::PORT_NO_INTEGER;
        return;
    }

    struct sockaddr_in serverStruct;
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(mServerSocketHandle < 0){
        result = NetworkError::SOCKET_FAILED;
        return;
    }

    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(shortport);
    serverStruct.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(mServerSocketHandle, (struct sockaddr *) &serverStruct, sizeof(serverStruct)) < 0){
        result = NetworkError::BIND_FAILED;
        return;
    }

    if(listen(mServerSocketHandle, 4) < 0){
        result = NetworkError::LISTEN_FAILED;
        return;
    }

    // successfully navigated the mine field
    result = NetworkError::ERROR_NO_ERROR;
}

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

int Server::send_(const NewMessage &d) {
    char* msgData = d.getOutData();
    int errCount = 0;

    // send to all
    if(d.getTarget().isAllPeer()) {
        for(int i = mClients->size() - 1; i >= 0; i--) {
            if(Network::noSignalSend(mClients->at(i).getSocket(), msgData, d.getOutDataLength()) != 0) {
                mClients->erase(mClients->begin() + i);
                errCount--;
            }
        }
    }
    // only send to one peer
    else {
        const int targetSocket = d.getTarget().getSocket();
        if(Network::noSignalSend(targetSocket, msgData, d.getOutDataLength()) != 0) {
            // remove the client that just failed to send
            auto iterator = std::remove_if(
                                mClients->begin(),
                                mClients->end(),
            [&targetSocket](const Peer& p) {
                return p.getSocket() == targetSocket;
            });
            mClients->erase(iterator);
            errCount--;
        }
    }

    // free memory
    delete[] msgData;

    return errCount;
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
        for(size_t i = 0; i < clientCount; i++){
            emitClientDiffList(*mClients, std::vector<Peer>());
        }
        // close everything else
        closeNetwork();
        return;
    }

    // handle incoming messages
    for(size_t i = 0; i < clientCount; i++) {
        if(structs[i].revents & POLLIN) {
            // create message object on stack
            QByteArray in = readNetwork(structs[i].fd);

            // if the message is empty, we know a dc happened
            if(in.isEmpty()) {
                // gracefully close socket
                close(structs[i].fd);

                std::vector<Peer> oldClients(*mClients);
                // remove peer from list
                mClients->erase(mClients->begin() + i);
                // notify lib user
                emitClientDiffList(*mClients, oldClients);
            }

            switch((PacketType) in.at(0)) {
            case PacketType::CLIENT_REGISTRATION: {
                send_(ServerDataForClientsMessage(*mClients));

                break;
            }
            case PacketType::DATA: {
                emit received(DataMessage(in));
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

void Server::checkForNewClients(pollfd structs[], size_t structLength) {
    // check for new clients now that we cant fuck up the vector anymore
    if(structs[structLength].revents & POLLIN) {
        struct sockaddr_in clientStruct;
        unsigned int clientLength = sizeof(clientStruct);
        int clientSocket = accept(mServerSocketHandle, (struct sockaddr *) &clientStruct, &clientLength);
        if(clientSocket >= 0) {
            static Peer p(clientSocket);
            std::vector<Peer> oldPeers = *mClients;
            mClients->push_back(Peer(p));
            emitClientDiffList(oldPeers, *mClients);

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
Client::Client(const QString& host, const QString& port, NetworkError& result, QObject *parent) : Network(parent)
{
    bool validPort;
    short shortport = port.toShort(&validPort);

    if(!validPort){
        result = NetworkError::PORT_NO_INTEGER;
        return;
    }

    if(!getHostAddress(host, mHost)){
        result = NetworkError::HOST_NOT_RESOLVED;
        return;
    }

    struct sockaddr_in peerDescription;
    mClientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mClientSocket < 0){
        result = NetworkError::SOCKET_FAILED;
        return;
    }

    peerDescription.sin_family = AF_INET;
    peerDescription.sin_port = htons(shortport);
    peerDescription.sin_addr.s_addr = mHost;

    if(Connector::myConnect(mClientSocket, (struct sockaddr *) &peerDescription, sizeof(peerDescription)) < 0){
        result = NetworkError::CONTACT_FAILED;
        return;
    }

    result = NetworkError::ERROR_NO_ERROR;
}

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

int Client::send_(const NewMessage &d) {
    // get data
    char* msgData = d.getOutData();
    int success = 0;

    // ignore all previous errors
    errno = 0;


    switch(send(mClientSocket, msgData, d.getOutDataLength(), MSG_NOSIGNAL)) {
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
    struct pollfd pollingStruct[1] = {{mClientSocket, POLLIN, 0}};

    // can we poll?
    if(poll(pollingStruct, 1, 2) < 0) {
        emitClientDiffList(mPeerList, std::vector<Peer>());
        closeNetwork();
    } else if(pollingStruct[0].revents & POLLIN) {
        QByteArray data = readNetwork(mClientSocket);

        if(data.length() == 0) {
            emitClientDiffList(mPeerList, std::vector<Peer>());
            closeNetwork();
        }

        switch((PacketType) data.at(0)) {
        case PacketType::SERVER_REGISTRATION_ANSWER: {
            ServerDataForClientsMessage sdfcm(data);
            emitClientDiffList(mPeerList, sdfcm.getPeerList());
            mPeerList = sdfcm.getPeerList();
            break;
        }
        case PacketType::DATA:
            emit received(DataMessage(data));
            break;
        default:
            break;
        }
    }
}

bool Client::getHostAddress(const QString hostName, long& address) {
    struct hostent * host;
    struct in_addr h_addr;

    host = gethostbyname(hostName.toStdString().c_str());

    if(host == NULL)
        return false;

    h_addr.s_addr = *((unsigned long *) host -> h_addr_list[0]);
    address = *((long *) host -> h_addr_list[0]);
    return true;
}

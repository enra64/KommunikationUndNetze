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

}

int Server::send(const DataMessage &d) {

}

int Server::send(const QString &m, const Peer &target) {

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

}

int Client::send(const DataMessage &d) {

}

int Client::send(const QString &m, const Peer &target) {

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

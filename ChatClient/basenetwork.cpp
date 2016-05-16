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

QString BaseNetwork::networkToString(int fd, size_t* rLength){
    size_t readLength = read(fd, mBuffer, sizeof(mBuffer));
    mBuffer[readLength] = '\0';
    if(rLength != nullptr)
        *rLength = readLength;
    return QString(mBuffer);
}

void BaseNetwork::checkForNewClients(struct pollfd structs[], int clientCount){
    // check for new clients now that we cant fuck up the vector anymore
    if(structs[clientCount].revents & POLLIN){
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

void BaseNetwork::onPoll(){
    switch(mConnectionState){
        case ConnectionState::SERVER:{
            // constantly need this shit
            size_t clientCount = mClients->size();

            // check client sockets + server socket for new connections
            struct pollfd structs[clientCount + 1];

            // fill polling struct array
            for(size_t i = 0; i < clientCount + 1; i++){
                structs[i].fd = i < clientCount ? mClients->at(i).getSocket() : mServerSocketHandle;
                structs[i].events = POLLIN;
                structs[i].revents = 0;
            }

            // run around screaming if we cant poll
            if(poll(structs, clientCount + 1, 2) < 0){
                // -> Polling error, everything disconnected!
                for(size_t i = 0; i < clientCount; i++)
                    emit disconnect(mClients->at(i), 0);
                // close everything else
                closeNetwork();
                return;
            }

            for(size_t i = 0; i < clientCount; i++){
                if(structs[i].revents & POLLIN){
                    // create message object on stack
                    Message m(networkToString(structs[i].fd, nullptr), mClients->at(i));

                    // if the message is empty, we know a dc happened
                    if(m.isEmpty()){
                        // gracefully close socket
                        close(structs[i].fd);
                        // notify lib user
                        emit disconnect(mClients->at(i), mClients->size() - 1);
                        // remove peer from list
                        mClients->erase(mClients->begin() + i);
                    }
                    else
                        // copy our message to the lib user
                        emit messageReceived(m);
                }
            }

            // check for new clients
            checkForNewClients(structs, clientCount);
            return;
        }
        case ConnectionState::CLIENT:{
            struct pollfd pollingStruct[1] = {{mServerSocketHandle, POLLIN, 0}};

            // can we poll?
            if(poll(pollingStruct, 1, 2) < 0){
                emit disconnect(Peer("Server", mServerSocketHandle), 0);
                closeNetwork();
            }
            else if(pollingStruct[0].revents & POLLIN){// yes we can
                // create new message
                Message m(networkToString(pollingStruct[0].fd, nullptr), Peer("Server", mServerSocketHandle));

                //message valid?
                if(!m.isEmpty())
                    emit messageReceived(m);
                else{
                    emit disconnect(Peer("Server", mServerSocketHandle), 0);
                    closeNetwork();
                }
            }
        }
        default:
            return;
    }
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

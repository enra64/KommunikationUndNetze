#include "highnetwork.h"

HighNetwork::HighNetwork(QObject *parent) : BaseNetwork(parent){
    mConnectionState = ConnectionState::NO_CONNECTION;

    QObject::connect(
                &mServerWaitWatcher,
                SIGNAL(finished()),
                this,
                SLOT(onAccept()));
}

int HighNetwork::asyncWaitForClients(struct sockaddr*){
    return await_contact(mPort);
}

void HighNetwork::onAccept(){
    int clientSocket = mServerWaitWatcher.result();
    if(clientSocket < 0){
        clientConnected(NetworkError::AWAIT_CONTACT_FAILED);
    }
    else{
        mClients->push_back(Peer(mServerWaitWatcher.result()));
        mServerSocketHandle = get_server_socket(mPort);
        clientConnected(NetworkError::ERROR_NO_ERROR);
    }
}

NetworkError HighNetwork::server(const QString port) {
    if(!parsePort(port, mPort))
        return NetworkError::PORT_NO_INTEGER;

    mServerWaitWatcher.cancel();

    mConnectionState = ConnectionState::SERVER;

    // start connection in concurrent thread
    QFuture<connection> future = QtConcurrent::run(this, &HighNetwork::asyncWaitForClients, nullptr);

    mServerWaitWatcher.setFuture(future);

    return NetworkError::ERROR_NO_ERROR;
}

NetworkError HighNetwork::client(const QString host, const QString port) {
    if(!parsePort(port, mPort))
        return NetworkError::PORT_NO_INTEGER;

    mHost = cname_to_comp(host.toLatin1().data());

    if(mHost == -1)
        return NetworkError::HOST_NOT_RESOLVED;

    if((mServerSocketHandle = make_contact(mHost, mPort)) < 0)
        return NetworkError::MAKE_CONTACT_FAILED;

    mConnectionState = ConnectionState::CLIENT;

    return NetworkError::ERROR_NO_ERROR;
}

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

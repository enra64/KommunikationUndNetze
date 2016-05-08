#include "highnetwork.h"

HighNetwork::HighNetwork(QObject *parent) : BaseNetwork(parent){
}

int HighNetwork::closeNetwork() {
    int ret = end_contact(mNetwork);

    emit disconnect("Disconnect!", 0);

    // reset member variables
    mZeroLengthMsgCount = 0;
    mNetwork = -1;
    mPort = -1;
    mHost = -1;

    return ret;
}

int HighNetwork::send(const QString msg) {
    // write to output
    int written = write(mNetwork, msg.toLatin1().data(), msg.length());
    if(written != msg.length())
        return -1;
    return 0;
}

int HighNetwork::onPoll(){
    // polling structs
    struct pollfd pollStructs[1] = {{mNetwork, POLLIN, 0}}; // poll network
    if(poll(pollStructs, 1, 2) < 0){
        if(closeNetwork() < 0)
            return -2;
        return -1;
    }
    if(pollStructs[0].revents & POLLIN){
        size_t readLength;
        QString mess = networkToString(pollStructs[0].fd, readLength);
        if(readLength != 0)
            emit messageReceived(Message(mess, "Not you: "));
        else{
            if(closeNetwork() < 0)
                return -4;
            return -3;
        }
    }
    return 0;
}

int HighNetwork::waitForClients(struct sockaddr*){
    // wait for connection
    return await_contact(mPort);
}

void HighNetwork::onAccept(){
    mNetwork = mServerWaitWatcher.result();
    clientConnected(mNetwork >= 0);
}

int HighNetwork::server(const QString port) {
    if(!parsePort(port, mPort))
        return -1;

    mServerWaitWatcher.cancel();

    // start connection in concurrent thread
    QFuture<connection> future = QtConcurrent::run(this, &HighNetwork::waitForClients, nullptr);

    mServerWaitWatcher.setFuture(future);

    return 0;
}

int HighNetwork::client(const QString host, const QString port) {
    if(!parsePort(port, mPort))
        return -1;

    mHost = cname_to_comp(host.toLatin1().data());

    if(mHost == -1){
        return -2;
    }
    mNetwork = make_contact(mHost, mPort);
    return mNetwork;
}

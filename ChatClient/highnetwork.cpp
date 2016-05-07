#include "highnetwork.h"

HighNetwork::HighNetwork(){
}

int HighNetwork::closeNetwork() {
    int ret = end_contact(mNetwork);

    // reset member variables
    mZeroLengthMsgCount = 0;
    mNetwork = -1;
    mPort = -1;
    mHost = -1;


    // follow up with ui
//    mChatWindow->connectionStatus(false);
//    mChatWindow->setSendingUiEnabled(false);
//    mChatWindow->setConnectionUiEnabled(true);

    return ret;
}

int HighNetwork::send(const QString msg) {
    // write to output
    int written = write(mNetwork, msg.toLatin1().data(), msg.length());
    if(written != msg.length())
        return -1;
    return 0;
}

size_t HighNetwork::receive(std::vector<QString>& msg) {
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
        if(readLength != 0){
            msg.push_back(mess);
        }
        else
            mZeroLengthMsgCount++;
    }

    if(mZeroLengthMsgCount > 100){
        closeNetwork();
        return -3;
    }
    return 0;
}

connection HighNetwork::waitAsServer(){
    // wait for connection
    return await_contact(mPort);
}

void HighNetwork::handleServerWaitFinished(){
    mNetwork = mServerWaitWatcher.result();
}

int HighNetwork::server(const QString port) {
    if(!parsePort(port, mPort))
        return -1;

    QObject::connect(
                &mServerWaitWatcher,
                SIGNAL(finished()),
                this,
                SLOT(handleServerWaitFinished()));

    //TODO: somehow include a callback to the ui

    // start connection in concurrent thread
    QFuture<connection> future = QtConcurrent::run(this, &HighNetwork::waitAsServer);

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

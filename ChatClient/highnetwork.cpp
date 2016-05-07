#include "highnetwork.h"

void HighNetwork::closeNetwork(){
    if(end_contact(mNetwork) == 0)
      mChatWindow->notify("connection successfully closed\n");
    else
      mChatWindow->error("connection could not be closed\n");

    // reset member variables
    mZeroLengthMsgCount = 0;
    mNetwork = -1;
    mPort = -1;
    mHost = -1;

    // follow up with ui
    mChatWindow->connectionStatus(false);
    mChatWindow->setSendingUiEnabled(false);
    mChatWindow->setConnectionUiEnabled(true);
}

void HighNetwork::send(const QString msg){
    // write to output
    write(mNetwork, msg.toLatin1().data(), msg.length());
}

size_t HighNetwork::receive(std::vector<QString>& msg){
    // polling structs
    struct pollfd pollStructs[1] = {{mNetwork, POLLIN, 0}}; // poll network
    if(poll(pollStructs, 1, 2) < 0){
        mChatWindow->error("Polling error, terminating.");
        closeNetwork();
    }
    if(pollStructs[0].revents & POLLIN){
        size_t readLength;
        QString mess = networkToString(pollStructs[0].fd, readLength);
        if(readLength != 0){
            msg.push_back(mess);
            mChatWindow->print(msg);
        }
        else
            mZeroLengthMsgCount++;
    }

    if(mZeroLengthMsgCount > 100){
        mChatWindow->error("Your peer seems to have closed the connection!");
        closeNetwork();
    }
}

connection HighNetwork::waitAsServer(){
    // wait for connection
    return await_contact(mPort);
}

void HighNetwork::handleServerWaitFinished(){
    mNetwork = mServerWaitWatcher.result();

    mChatWindow->connectionStatus(mNetwork != -1);

    if(mNetwork == -1)
        mChatWindow->error("No client found within timeout!");
    else
        mChatWindow->notify("A Client connected!");
}

int HighNetwork::server(const QString port){
    if(!parsePort(port))
        return;

    QObject::connect(
                &mServerWaitWatcher,
                SIGNAL(finished()),
                this,
                SLOT(handleServerWaitFinished()));

    // notify user
    mChatWindow->notify("Waiting for connection...");
    // start connection in concurrent thread
    QFuture<connection> future = QtConcurrent::run(this, &Network::waitAsServer);

    mServerWaitWatcher.setFuture(future);

    return 0;
}

int HighNetwork::client(const QString host, const QString port){
    if(!parsePort(port)) return;

    mHost = cname_to_comp(host.toLatin1().data());

    if(mHost == -1){
        mChatWindow->error("Bad host entered");
        return -2;
    }

    mNetwork = make_contact(mHost, mPort);

    mChatWindow->connectionStatus(mNetwork != -1);

    if(mNetwork == -1)
        mChatWindow->error("No Server found");
    else
        mChatWindow->notify("Server found!");
    return mNetwork;
}

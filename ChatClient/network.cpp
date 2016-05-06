#include "network.h"

Network::Network(ChatWindow* cw)
{
    mChatWindow = cw;
}

void Network::closeNetwork(){
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

void Network::send(const QString msg){
    // write to output
    write(mNetwork, msg.toLatin1().data(), msg.length());
}

void Network::pollingRead(){
    // polling structs
    struct pollfd pollStructs[1] = {
      //{0, POLLIN, 0}, // poll stdin
      {mNetwork, POLLIN, 0}}; // poll network
    if(poll(pollStructs, 1, 2) < 0){
        mChatWindow->error("Polling error, terminating.");
        closeNetwork();
    }
    if(pollStructs[0].revents & POLLIN){
        QString msg("");
        if(networkToString(msg) != 0)
            mChatWindow->print(msg);
        else
            mZeroLengthMsgCount++;
    }

    if(mZeroLengthMsgCount > 100){
        mChatWindow->error("Your peer seems to have closed the connection!");
        closeNetwork();
    }
}

size_t Network::networkToString(QString& msg){
    size_t receiveLength;
    receiveLength = read(mNetwork, mBuffer, sizeof(mBuffer));
    mBuffer[receiveLength] = 0;
    msg.append(mBuffer);
    return receiveLength;
}

connection Network::waitAsServer(){
    // wait for connection
    return await_contact(mPort);
}

void Network::handleServerWaitFinished(){
    mNetwork = mServerWaitWatcher.result();

    mChatWindow->connectionStatus(mNetwork != -1);

    if(mNetwork == -1)
        mChatWindow->error("No client found within timeout!");
    else
        mChatWindow->notify("A Client connected!");
}

void Network::server(const QString port){
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
}

bool Network::parsePort(const QString port){
    bool portIsInt;
    mPort = port.toInt(&portIsInt);

    if(!portIsInt)
        mChatWindow->error("Port does not look very integer-y?");
    return portIsInt;
}

void Network::client(const QString host, const QString port){
    if(!parsePort(port)) return;

    mHost = cname_to_comp(host.toLatin1().data());

    if(mHost == -1){
        mChatWindow->error("Bad host entered");
        return;
    }

    mNetwork = make_contact(mHost, mPort);

    mChatWindow->connectionStatus(mNetwork != -1);

    if(mNetwork == -1)
        mChatWindow->error("No Server found");
    else
        mChatWindow->notify("Server found!");
}

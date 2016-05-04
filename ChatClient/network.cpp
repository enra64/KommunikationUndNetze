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
}

void Network::send(const QString msg){
    char* msgBuffer = msg.toLatin1().data();

    // write to output
    write(mNetwork, msgBuffer, msg.length());
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
    if(pollStructs[0].revents & POLLIN)
      mChatWindow->print(networkToString());
}

QString Network::networkToString(){
    size_t receiveLength;
    receiveLength = read(mNetwork, mBuffer, sizeof(mBuffer));
    mBuffer[receiveLength] = 0;
    return QString(mBuffer);
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
        mChatWindow->error("Port is not an integer!");
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

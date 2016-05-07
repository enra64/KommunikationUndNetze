#include "basenetwork.h"

BaseNetwork::BaseNetwork(QObject *parent) : QObject(parent)
{
    QObject::connect(
                &mServerWaitWatcher,
                SIGNAL(finished()),
                this,
                SLOT(onAccept()));
}

bool BaseNetwork::parsePort(const QString port, short& shortPort){
    bool portIsInt;
    shortPort = port.toInt(&portIsInt);
    return portIsInt;
}

QString BaseNetwork::networkToString(int fd, size_t& rLength){
    rLength = read(fd, mBuffer, sizeof(mBuffer));
    mBuffer[rLength] = '\0';
    return QString(mBuffer);
}

int BaseNetwork::waitForClients(struct sockaddr*){
    return -1234;
}

void BaseNetwork::onAccept(){
}

// dont ask me why this shit compiles

int BaseNetwork::client(const QString, const QString){
    return -1234;
}

int BaseNetwork::server(const QString){
    return -1234;
}

int BaseNetwork::send(const QString){
    return -1234;
}

size_t BaseNetwork::receive(std::vector<Message>&){
    return -1234;
}

int BaseNetwork::closeNetwork(){
    return -1234;
}

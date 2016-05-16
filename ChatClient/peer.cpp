#include "peer.h"

Peer::Peer(QString name, int socket) : mSocket(socket), mName(name)
{
    mIsValid = true;
}

Peer::Peer(int socket) : mSocket(socket)
{
    mName = QString("The Client using Socket %1").arg(socket);
    mIsValid = true;
}

Peer::Peer(){
    mIsValid = false;
}

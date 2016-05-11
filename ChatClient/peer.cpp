#include "peer.h"

Peer::Peer(QString name, int socket) : mSocket(socket), mName(name)
{

}

Peer::Peer(int socket) : mSocket(socket)
{
    mName = QString("The Client using Socket %1").arg(socket);
}

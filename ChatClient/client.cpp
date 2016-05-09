#include "client.h"

Peer::Peer(QString name, int socket) : mSocket(socket), mName(name)
{

}

Peer::Peer(int socket) : mSocket(socket)
{
    mName = QString("Client %1").arg(socket);
}

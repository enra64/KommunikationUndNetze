#include "basenetwork.h"

BaseNetwork::BaseNetwork(QObject *parent) : QObject(parent)
{

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

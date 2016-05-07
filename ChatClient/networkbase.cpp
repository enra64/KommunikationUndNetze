#include "networkbase.h"

NetworkBase::NetworkBase(ChatWindow &cw)
{
    mChatWindow = cw;
}

bool NetworkBase::parsePort(const QString port){
    bool portIsInt;
    mPort = port.toInt(&portIsInt);
    return portIsInt;
}

QString NetworkBase::networkToString(int fd, size_t& rLength){
    rLength = read(fd, mBuffer, sizeof(mBuffer));
    mBuffer[rLength] = '\0';
    return QString(mBuffer);
}

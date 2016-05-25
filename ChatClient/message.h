#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include "peer.h"

enum struct PacketType {
    CLIENT_REGISTRATION,
    SERVER_REGISTRATION_ANSWER,
    MESSAGE
};

class Message
{
public:
    Message(QString msg, Peer sender);
    Message(QString msg, Peer sender, Peer target);

    inline int length()
    {
        return mMessage.length();
    }

    inline bool isEmpty()
    {
        return mMessage.isEmpty();
    }

    inline bool getDataLength(){
        return
                (mTarget.isAllPeer() ? 0 : 4) + // 4 bytes for target socket int
                4 + // 4 bytes for packet info
                mMessage.length(); // enough bytes for the message data
    }

    inline bool getData(char* buf, size_t len){
        if(len < getDataLength())
            return false;
        buf[0] = (unsigned char) mPacketType;

    }

    inline QString getMessage(){
        return mMessage;
    }

    inline Peer getSender(){
        return mSender;
    }

private:
    PacketType mPacketType;
    QString mMessage;
    Peer mSender;
    Peer mTarget;
};

#endif // MESSAGE_H

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include "peer.h"

class Message
{
public:
    Message(QString msg, Peer sendr);

    inline int length()
    {
        return mMessage.length();
    }

    inline bool isEmpty()
    {
        return mMessage.isEmpty();
    }

    inline QString getMessage(){
        return mMessage;
    }

    inline Peer getSender(){
        return mSender;
    }

private:
    QString mMessage;
    Peer mSender;
};

#endif // MESSAGE_H

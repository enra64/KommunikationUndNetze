#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

class Peer
{
public:
    Peer(QString name, int socket);
    Peer(int socket);
    Peer();

    inline Peer createAllPeer(){
        return Peer(0);
    }

    inline bool isAllPeer(){
        return mSocket == 0;
    }

    inline int getSocket(){
        return mSocket;
    }

    inline QString getName(){
        return mName;
    }

private:
    bool mIsValid;
    int mSocket;
    QString mName;
};

#endif // CLIENT_H

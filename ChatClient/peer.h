#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

class Peer
{
public:
    Peer(QString name, int socket);
    Peer(int socket);
    Peer();

    inline bool isSelfPeer(int socket) const {
        return mSocket == socket;
    }

    inline bool isAllPeer() const {
        return mSocket == 0;
    }

    inline int getSocket() const {
        return mSocket;
    }

    inline QString getName() const {
        return mName;
    }

private:
    bool mIsValid;
    int mSocket;
    QString mName;
};

#endif // CLIENT_H

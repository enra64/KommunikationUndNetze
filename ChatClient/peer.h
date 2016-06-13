#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

class Peer
{
public:
    Peer(){
        mIsValid = false;
    }

    Peer(int socket):
        mSocket(socket) {
        mName = QString("The Client using Socket %1").arg(socket);
    }

    Peer(QString name, int socket) :
        mSocket(socket),
        mName(name) {

    }

    inline bool isSelfPeer(int socket) const {
        return mSocket == socket;
    }

    static inline const Peer getAllPeer() {
        return Peer(0);
    }

    static inline const Peer getServerPeer(){
        return Peer(-12);
    }

    inline bool isAllPeer() const {
        return mSocket == 0;
    }

    inline bool isServerPeer() const {
        return mSocket == -12;
    }

    inline int getSocket() const {
        return mSocket;
    }

    inline QString getName() const {
        return mName;
    }

private:
    bool mIsValid = true;
    int mSocket;
    QString mName;
};

#endif // CLIENT_H

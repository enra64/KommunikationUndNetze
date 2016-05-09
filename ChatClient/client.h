#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

class Peer
{
public:
    Peer(QString name, int socket);
    Peer(int socket);

    inline int getSocket(){
        return mSocket;
    }

    inline QString getName(){
        return mName;
    }

private:
    int mSocket;
    QString mName;
};

#endif // CLIENT_H

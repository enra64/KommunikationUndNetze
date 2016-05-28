#ifndef NETWORK_H
#define NETWORK_H

#include <QByteArray>
#include <QtConcurrent/QtConcurrent>

#include "peer.h"
#include "newmessage.h"
#include "compilerdistractor.h"

#include <netinet/in.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>          /* hostent struct, gethostbyname() */

#include <vector>

enum struct NetworkError{
    ERROR_NO_ERROR,
    PORT_NO_INTEGER,
    AWAIT_CONTACT_FAILED,
    BIND_FAILED,
    LISTEN_FAILED,
    ACCEPT_FAILED,
    SOCKET_FAILED,
    CONTACT_FAILED,
    HOST_NOT_RESOLVED,
    MAKE_CONTACT_FAILED
};

class Network : public QObject {
    Q_OBJECT
public:
    explicit Network(QObject *parent = 0);

    /// dtor, should close the network.
    //~Network()

    /// Closes the network and clears all variables, effectively returning to a new state.
    virtual int closeNetwork() = 0;

    /// Send the datamessage
    virtual int send_(const NewMessage& d) = 0;

    /// send to all peers
    inline int send_(const QString& m) {
        return send_(m, Peer::getAllPeer());
    }

    /// send to specific peer
    inline int send_(const QString &m, const Peer &target) {
        return send_(DataMessage(m, Peer(getNetworkId()), target));
    }

    /// uses the network signal to
    void emitClientDiffList(const std::vector<Peer> &oldPeers, const std::vector<Peer> &newPeers);

protected:
    /// send ignoring sigpipe
    int noSignalSend(int socket, char* data, size_t dataLength);

    /// reports the id representing the network object
    virtual int getNetworkId() = 0;

    /// Reads network input to a byte array
    QByteArray readNetwork(int fd);

    /// Parse a qstring to a port short
    bool parsePort(const QString& portIn, short& portOut);

    /// buffer for all operations requiring a c-style buffer
    char mBuffer[1024];

    /// QTimer for onPoll
    QTimer* mTimer;

protected slots:
    /// Called every x ms to give the network time to react
    virtual void onPoll() = 0;

signals:
    /// Called whenever a data message has been received
    void received(const DataMessage& d);

    /// Called whenever a peer has been added or removed
    void peerListUpdated(std::vector<Peer> peerList, std::vector<Peer> removedPeerList, int sizeDiffSize);

    /// Called when the network has been closed
    void networkClosed(int status);
};

/// Server class implementing the network interface
class Server : public Network {
    Q_OBJECT
public:
    Server(const QString& port, NetworkError &result, QObject *parent = 0);
    int closeNetwork() override;
    int send_(const NewMessage &d) override;

signals:
    void received(const DataMessage &d);
    void networkClosed(int status);

protected:
    /// reports the id representing the network object
    int getNetworkId() override {
        return mServerSocketHandle;
    }

protected slots:
    void onPoll() override;

private:
    /// list of current clients
    std::vector<Peer>* mClients;

    /// socket handle in use for server
    int mServerSocketHandle = -1;

    /// port we are listening on
    short mPort = -1;

    /// checks for new clients
    void checkForNewClients(struct pollfd structs[], size_t structLength);
};

/// Client class implementing the network interface
class Client : public Network {
    Q_OBJECT
public:
    explicit Client(const QString &host, const QString &port, const QString &name, NetworkError &result, QObject *parent = 0);
    int closeNetwork() override;
    int send_(const NewMessage &d) override;

signals:
    void received(const DataMessage &d);
    void networkClosed(int status);

protected:
    /// reports the id representing the network object
    inline int getNetworkId() override {
        return mNetworkId;
    }

protected slots:
    void onPoll() override;

private:
    /// port we are listening on
    short mPort = -1;

    /// host we are using
    long mHost = -1;

    /// id the server knows us as
    int mNetworkId = -1;

    /// our socket
    int mClientSocket = -1;

    /// save old peer list
    std::vector<Peer> mPeerList;

    /// convert a string to a long address
    bool getHostAddress(const QString hostName, long& address);
};

#endif // NETWORK_H

#ifndef NETWORK_H
#define NETWORK_H

#include <QByteArray>

#include "peer.h"

#include <vector>

class Network : public QObject {
    Q_OBJECT
public:
    explicit Network(QObject *parent = 0);

    /// dtor, should close the network.
    //~Network()

    /// Closes the network and clears all variables, effectively returning to a new state.
    virtual int closeNetwork() = 0;

    /// Send the datamessage
    virtual int send(const DataMessage& d) = 0;

    /// Create a data message and send it
    virtual int send(const QString& m, const Peer& target) = 0;

protected:
    /// Reads network input to a byte array
    explicit QByteArray readNetwork(int fd, size_t* receiveLength);

    /// Parse a qstring to a port short
    explicit bool parsePort(const QString& port, short& port);

    /// buffer for all operations requiring a c-style buffer
    char mBuffer[1024];

signals:
    /// Called whenever a data message has been received
    virtual void received(const DataMessage& d) = 0;

    /// Called whenever a peer has been added or removed
    virtual void peerListUpdated(std::vector<Peer> peerList) = 0;

    /// Called when the network has been closed
    virtual void networkClosed(int status) = 0;

protected slots:
    /// Called every x ms to give the network time to react
    virtual void onPoll() = 0;
};

/// Server class implementing the network interface
class Server : public Network {
    Q_OBJECT
public:
    int closeNetwork() override;
    int send(const DataMessage &d) override;
    int send(const QString& m, const Peer& target) override;

signals:
    void received(const DataMessage &d) override;
    void peerListUpdated(std::vector<Peer> peerList) override;
    void networkClosed(int status) override;
private slots:
    void onPoll() override;
};

/// Client class implementing the network interface
class Client : public Network {
    Q_OBJECT
public:
    int closeNetwork() override;
    int send(const DataMessage &d) override;
    int send(const QString& m, const Peer& target) override;

signals:
    void received(const DataMessage &d) override;
    void peerListUpdated(std::vector<Peer> peerList) override;
    void networkClosed(int status) override;
private slots:
    void onPoll() override;
};

#endif // NETWORK_H

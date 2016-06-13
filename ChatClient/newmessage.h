#ifndef NEWMESSAGE_H
#define NEWMESSAGE_H

#include "peer.h"
#include <stdio.h>
#include <vector>
#include <string>

#include <QString>

enum struct PacketType {
    CLIENT_REGISTRATION,
    SERVER_REGISTRATION_ANSWER,
    DATA
};

class NewMessage
{
public:
    NewMessage(PacketType packetType) : mPacketType(packetType){

    }

    virtual QByteArray* getOutData() const = 0;

    virtual bool isEmpty() const {
        return true;
    }

    inline const Peer& getTarget() const {
        return mTarget;
    }

    inline const Peer& getSource() const {
        return mSource;
    }

    inline const PacketType& getType() const {
        return mPacketType;
    }

protected:
    /// write an int to a char buffer
    /// pretty sure this is 300% efficient and does not do 13 copies
    static QByteArray intToByteArray(int i) {
        char tmp[5];
        snprintf(tmp, 5, "%i", i);
        return QByteArray(tmp, 5);
    }

protected:
    PacketType mPacketType;
    Peer mTarget, mSource;
};

/// Message the client sends to the server to make its name known
/// layout:
/// first byte: packet type
/// all other bytes: name
class ClientRegistrationMessage : public NewMessage
{
public:
    ClientRegistrationMessage(QString name) : NewMessage(PacketType::CLIENT_REGISTRATION), mName(name) {
    }

    ClientRegistrationMessage(QByteArray& in) : NewMessage(PacketType::CLIENT_REGISTRATION) {
        char* data = in.data();
        mName = QString(data + 1); // pray that it is nullterminated
    }

    QByteArray* getOutData() const override {
        QByteArray* a = new QByteArray();
        a->append((char) mPacketType);
        a->append(mName);
        return a;
    }

    bool isEmpty() const {
        return mName.isEmpty();
    }

private:
    QString mName;
};



/// Message the server sends to all clients whenever a new client has connected
/// layout:
/// first byte: packet type
/// byte 1 to x: all peers, with the following layout:
/// socket - name length - name
class ServerDataForClientsMessage : public NewMessage
{
public:
    ServerDataForClientsMessage(std::vector<Peer> peers) :
        NewMessage(PacketType::SERVER_REGISTRATION_ANSWER),
        mPeerList(peers) {
    }

    ServerDataForClientsMessage(QByteArray& in) : NewMessage(PacketType::SERVER_REGISTRATION_ANSWER) {
        char* data = in.data();
        char* end = data + in.length();
        mPeerList = std::vector<Peer>();
        // ignore message type
        data++;
        // does this look spooky to you? it is.
        while(data < end){
            // read socket, increase pointer
            int socket = atoi(data);
            data += 5;

            // read name length, increase pointer
            int nameLength = atoi(data);
            data += 5;

            // read name, increase pointer
            QString name (data);
            data += nameLength + 1;

            // save new client
            mPeerList.push_back(Peer(name, socket));
        }
    }

    bool isEmpty() const {
        return mPeerList.empty();
    }

    std::vector<Peer> getPeerList(){
        return mPeerList;
    }

    QByteArray* getOutData() const override {
        QByteArray* a = new QByteArray;
        a->append((char) mPacketType);
        for(Peer p : mPeerList){
            a->append(intToByteArray(p.getSocket()));
            a->append(intToByteArray(p.getName().size()));
            a->append(p.getName());
        }
        return a;
    }

    std::vector<Peer> mPeerList;
};



/// Message sent by server or client when data shall be transmitted
/// layout:
/// first byte: pkt type
/// byte 1 to 5: source socket (null terminated itoa int)
/// byte 6 to 10: target socket (null terminated itoa int)
/// byte 11 to x: data
class DataMessage : public NewMessage
{
public:
    DataMessage(QString data, Peer source, Peer target) :
        NewMessage(PacketType::DATA),
        mData(data) {
        mSource = source;
        mTarget = target;
    }

    DataMessage(QByteArray& in) : NewMessage(PacketType::DATA){
        char* data = in.data();
        // ignore message type
        data++;
        mSource = atoi(data);
        data += 5;

        mTarget = atoi(data);
        data += 5;

        mData = QString(data);
    }

    const QString& getMessage() const {
        return mData;
    }

    QByteArray* getOutData() const override {
        QByteArray* data = new QByteArray();
        data->append((char) mPacketType);
        data->append(intToByteArray(mSource.getSocket()));
        data->append(intToByteArray(mTarget.getSocket()));
        data->append(mData);
        return data;
    }

    inline int length() { return mData.length(); }
    inline bool isEmpty() { return mData.isEmpty(); }

private:
    QString mData;
};

#endif // NEWMESSAGE_H

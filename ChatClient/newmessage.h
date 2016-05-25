#ifndef NEWMESSAGE_H
#define NEWMESSAGE_H

#include "peer.h"
#include <QString>

enum struct PacketType {
    CLIENT_REGISTRATION,
    SERVER_REGISTRATION_ANSWER,
    MESSAGE
};

class NewMessage
{
public:
    NewMessage(PacketType packetType){

    }

    virtual const char* getData();

    virtual const size_t getLength();
private:
    PacketType mPacketType;
    Peer mTarget;
};

class ClientRegistrationMessage : public NewMessage
{
public:
    ClientRegistrationMessage(QString name) : NewMessage(PacketType::CLIENT_REGISTRATION), mName(name) {

    }

    char* getData() override {
        QByteArray& name = mName.toLatin1().prepend((unsigned char) mName.length()).prepend((unsigned char) mPacketType);
        return name.data();
    }

    const size_t getLength() override {
        return 2 + mName.length();
    }

private:
    QString mName;
};

class ServerDataForClientsMessage : public NewMessage
{
public:
    ServerDataForClientsMessage(std::vector<Peer>& peers) :
        NewMessage(PacketType::SERVER_REGISTRATION_ANSWER),
        mPeerList(peers){

    }

    const char* getData() override {
        QByteArray a;
        char intBuf[4];
        for(Peer p : mPeerList){
            snprintf(intBuf, 4, "%d", p.getName().length());
            a.append(intBuf, 4);

            a.append(p.getName());

            snprintf(intBuf, 4, "%d", p.getSocket());
            a.append(intBuf, 4);
        }
        return a.data();
    }

    const size_t getLength() override {
        size_t len;
        for(Peer p : mPeerList)
            len += 8 + p.getName().length();
        return len;
    }

    std::vector<Peer> mPeerList;
};

class DataMessage : public NewMessage
{
public:
    DataMessage(QString data, Peer source) :
        mData(data),
        mSource(source) {
        mHasTarget = false;
    }

    DataMessage(QString data, Peer source, Peer target) :
        mData(data),
        mSource(source),
        mTarget(target){
        mHasTarget = true;
    }

    inline bool hasTarget(){
        return mHasTarget;
    }

    const char* getData() override {

    }

    const size_t getLength() override {

    }

    inline int length()
    {
        return mData.length();
    }

    inline bool isEmpty()
    {
        return mData.isEmpty();
    }

private:
    QString mData;
    Peer mSource;
    bool mHasTarget = false;
};



#endif // NEWMESSAGE_H

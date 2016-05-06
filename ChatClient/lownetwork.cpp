#include "lownetwork.h"

bool LowNetwork::getHostAddress(QString hostName, unsigned long& address){
    struct hostent * host;
    struct in_addr h_addr;

    host = gethostbyname(hostName.toStdString().c_str());

    if(host == NULL)
        return false;

    h_addr.s_addr = *((unsigned long *) host -> h_addr_list[0]);
    return true;
}

bool LowNetwork::getSocket(int& socketHandle, unsigned long address, int port){
    struct sockaddr_in peerDescription;
    socketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketHandle < 0)
        return false;
    peerDescription.sin_family = AF_INET;
    peerDescription.sin_addr.s_addr = address;
    peerDescription.sin_port = htons(port);

    return connect(socketHandle, (struct sockaddr *) &peerDescription, sizeof(peerDescription)) >= 0;
}

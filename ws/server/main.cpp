#include <iostream>

#include "header.h"
#include "enums.h"
#include "requestresponder.h"

#include <vector>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace std;

short mPort;
unsigned long mHost;
int mServerSocket;

vector<thread> mResponderThreads;

int closeNetwork(){
    int err = 0;

    if(close(mServerSocket) < 0)
        err--;

    mHost = -1;
    mPort = -1;
    mServerSocket = -1;

    return err;
}

void respond(int socket){
    RequestResponder responder(socket);
    responder.respond();
}

NetworkEvent poll(){
    // check client sockets + server socket for new connections
    struct pollfd pollingStruct[1] = {{mServerSocket, POLLIN, 0}};

    // run around screaming if we cant poll
    if(poll(pollingStruct, 1, -1) < 0){
        // close everything else
        closeNetwork();
        return NetworkEvent::POLLING_ERROR;
    }

    // check for new clients
    if(pollingStruct[0].revents & POLLIN){
        struct sockaddr_in clientStruct;
        unsigned int clientLength = sizeof(clientStruct);
        int clientSocket = accept(mServerSocket, (struct sockaddr *) &clientStruct, &clientLength);

        mResponderThreads.push_back(thread(respond, clientSocket));
    }
    return NetworkEvent::NO_ERROR;
}

NetworkError server(int argc, char *argv[]){
    if(argc < 2)
        mPort = 2047;
        //return NetworkError::PORT_NO_INTEGER;
    else
        mPort = atoi(argv[1]);
    mHost = htonl(INADDR_ANY);

    struct sockaddr_in serverStruct;
    mServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(mServerSocket < 0)
        return NetworkError::SOCKET_FAILED;

    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(mPort);
    serverStruct.sin_addr.s_addr = mHost;

    int param = 1;
    if(setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, &param, sizeof(param)) < 0)
        return NetworkError::SET_SOCK_OPT_FAILED;

    if(bind(
        mServerSocket,
        (struct sockaddr *) &serverStruct,
        sizeof(serverStruct)) < 0)
        return NetworkError::BIND_FAILED;

    if(listen(mServerSocket, 200) < 0)
        return NetworkError::LISTEN_FAILED;

    while(1){
        if(poll() != NetworkEvent::NO_ERROR){
            closeNetwork();
            return NetworkError::POLL_FAILED;
        }
    }

    cout << "joining threads" << endl;
    for(int i = 0; i < mResponderThreads.size(); i++)
        mResponderThreads[i].join();
}

int main(int argc, char *argv[])
{
    switch(server(argc, argv)){
        case NetworkError::POLL_FAILED:
            cout << "poll() failed" << endl;
        break;
        case NetworkError::LISTEN_FAILED:
            cout << "listen() failed"  << endl;
            break;
        case NetworkError::BIND_FAILED:
            cout << "bind() failed"  << endl;
            break;
        case NetworkError::SOCKET_FAILED:
            cout << "socket() failed"  << endl;
            break;
        case NetworkError::PORT_NO_INTEGER:
            cout << "port no integer or not given" << endl;
            break;
        case NetworkError::ERROR_NO_ERROR:
            cout << "no error" << endl;
            break;
        case NetworkError::ACCEPT_FAILED:
            cout << "accept() failed" << endl;
            break;
        case NetworkError::SET_SOCK_OPT_FAILED:
            cout << "could not change socket options" << endl;
            break;
    }

    return 0;
}

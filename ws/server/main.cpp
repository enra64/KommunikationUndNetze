#include <iostream>

#include "httpheader.h"
#include "requestresponder.h"

#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>
#include <unistd.h>
#include <vector>
#include <signal.h>
#include <thread>

enum struct NetworkError{
    ERROR_NO_ERROR,
    PORT_NO_INTEGER,
    BIND_FAILED,
    LISTEN_FAILED,
    ACCEPT_FAILED,
    SOCKET_FAILED,
    POLL_FAILED,
    SET_SOCK_OPT_FAILED
};

using namespace std;

int mServerSocket;
struct pollfd pollingStruct[1] = {{-1, POLLIN, 0}};
vector<thread> mResponderThreads;

void closeNetwork(){
    cout << "\nclosing socket and joining all responder threads" << endl;
    for(size_t i = 0; i < mResponderThreads.size(); i++)
        mResponderThreads[i].join();
    close(mServerSocket);
}

void sig_handler(int signal){
    if(signal != SIGINT)
        return;
    closeNetwork();
    exit(0);
}

NetworkError poll(){
   // cant poll? run around screaming.
    if(poll(pollingStruct, 1, -1) < 0)
        return NetworkError::POLL_FAILED;

    // check for new clients
    if(pollingStruct[0].revents & POLLIN){
        int clientSocket = accept(mServerSocket, nullptr, nullptr);

        if(clientSocket >= 0)
            mResponderThreads.push_back(thread(RequestResponder::handle, clientSocket));
        else
            return NetworkError::ACCEPT_FAILED;
    }
    return NetworkError::ERROR_NO_ERROR;
}

NetworkError server(int argc, char *argv[]){
    // read port or use default
    short port = argc < 2 ? 2047 : atoi(argv[1]);

    // get a socket
    if((mServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        return NetworkError::SOCKET_FAILED;

    struct sockaddr_in serverStruct;
    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(port);
    serverStruct.sin_addr.s_addr = htonl(INADDR_ANY);

    int enableAddressReuse = 1;
    if(setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, &enableAddressReuse, sizeof(enableAddressReuse)) < 0)
        return NetworkError::SET_SOCK_OPT_FAILED;

    if(bind(mServerSocket, (struct sockaddr *) &serverStruct, sizeof(serverStruct)) < 0)
        return NetworkError::BIND_FAILED;

    if(listen(mServerSocket, 200) < 0)
        return NetworkError::LISTEN_FAILED;

    pollingStruct[0].fd = mServerSocket;

    while(poll() == NetworkError::ERROR_NO_ERROR);

    return NetworkError::POLL_FAILED;
}

int main(int argc, char *argv[])
{
    // handle sigint
    signal(SIGINT, sig_handler);

    // start server
    switch(server(argc, argv)) {
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
        case NetworkError::ACCEPT_FAILED:
            cout << "accept() failed" << endl;
            break;
        case NetworkError::SET_SOCK_OPT_FAILED:
            cout << "could not change socket options" << endl;
            break;
        case NetworkError::ERROR_NO_ERROR:
            cout << "no error" << endl;
            break;
    }

    closeNetwork();
    perror("Error");

    return 0;
}

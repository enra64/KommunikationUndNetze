#include <iostream>

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

char buffer[4096];

enum struct NetworkEvent{
    POLLING_ERROR,
    NO_ERROR
};

enum struct NetworkError{
    ERROR_NO_ERROR,
    PORT_NO_INTEGER,
    BIND_FAILED,
    LISTEN_FAILED,
    ACCEPT_FAILED,
    SOCKET_FAILED,
    POLL_FAILED
};

int closeNetwork(){
    int err = 0;
    
    if(close(mServerSocket) < 0)
        err--;

    mHost = -1;
    mPort = -1;
    mServerSocket = -1;

    return err;
}

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}

FILE* getWwwFilePath(char* httpPath){
    int pLen = strlen(httpPath);
    char buffer[256] = "htdocs";
    memcpy(buffer + 6, httpPath, pLen);
    return fopen(buffer, "r");
}

void clientConnected(NetworkError e, int socket){
    if(e == NetworkError::ERROR_NO_ERROR){
        int readSize;
        FILE* file;
        // parse out what file we want
        readSize = read(socket, buffer, sizeof(buffer));

        // the buffer *should* now contain "GET <path...> HTTP..."
        //                                  01234
        if(buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T'){
            if(buffer[4] == '/' && buffer[5] == ' '){
                file = getWwwFilePath("/index.http");
            }
        }
        else{
            write(socket, "are you a browser?", 18);
            close(socket);
            return;
        }

        // give the file back
        do{
            readSize = fread(buffer, 1, sizeof(buffer), file);
            if(readSize <= 0)
                break;
            // try to send everything in that buff
            if(sendall(mServerSocket, buffer, &readSize) == -1){
                cout << "could only send" << readSize << endl;
            }
        } while(readSize == sizeof(buffer));
    }
    close(socket);
}

NetworkEvent poll(){
    // check client sockets + server socket for new connections
    struct pollfd pollingStruct[1] = {{mServerSocket, POLLIN, 0}};

    // run around screaming if we cant poll
    if(poll(pollingStruct, 1, 0) < 0){
        // close everything else
        closeNetwork();
        return NetworkEvent::POLLING_ERROR;
    }

    // check for new clients
    if(pollingStruct[0].revents & POLLIN){
        struct sockaddr_in clientStruct;
        unsigned int clientLength = sizeof(clientStruct);
        int clientSocket = accept(mServerSocket, (struct sockaddr *) &clientStruct, &clientLength);

        if(clientSocket < 0){
            clientConnected(NetworkError::ACCEPT_FAILED, -1);
        }
        else{
            clientConnected(NetworkError::ERROR_NO_ERROR, clientSocket);
        }
    }
    return NetworkEvent::NO_ERROR;
}

NetworkError server(int argc, char *argv[]){
    if(argc < 2)
        mPort = 2048;
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

    if(bind(
        mServerSocket,
        (struct sockaddr *) &serverStruct,
        sizeof(serverStruct)) < 0)
        return NetworkError::BIND_FAILED;

    if(listen(mServerSocket, 4) < 0)
        return NetworkError::LISTEN_FAILED;



    while(1){
        if(poll() != NetworkEvent::NO_ERROR){
            closeNetwork();
            return NetworkError::POLL_FAILED;
        }
    }
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
    }

    return 0;
}

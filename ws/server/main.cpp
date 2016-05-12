#include <iostream>

#include "header.h"

#include <vector>
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

char buffer[1024];

enum struct ResponseType{
    NOT_FOUND,
    BAD_REQUEST,
    OK
};

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
    POLL_FAILED,
    SET_SOCK_OPT_FAILED
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

int sendall(int s, char *buf, int *len){
    int sentBytes = 0;        // how many bytes we've sent
    int bytesLeftToSend = *len; // how many we have left to send
    int tempBytesSent;

    while(sentBytes < *len) {
        tempBytesSent = send(s, buf+sentBytes, bytesLeftToSend, 0);
        if (tempBytesSent == -1)
            break;
        sentBytes += tempBytesSent;
        bytesLeftToSend -= tempBytesSent;
    }

    *len = sentBytes; // return number actually sent here

    return tempBytesSent == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

string parsePath(string line){
    // find end of requested file path
    size_t httpLoc = line.find(" HTTP/");

    // truncate to path
    line.resize(httpLoc);

    // remove GET and prepend htdocs
    string path = "htdocs" + string(line, 4);

    // send index if no file is specified
    if(path.length() == 7)
        path = "htdocs/index.html";

    return path;
}

void parseHeaders(vector<Header>& singleHeaderList, string& header, bool& headerOk){
    size_t delimiterPosition;
    while((delimiterPosition = header.find("\r\n")) != string::npos){
        string line = header.substr(0, delimiterPosition);
        if(line.find("GET ") == 0)
            singleHeaderList.push_back(Header("GET", parsePath(line)));
        else
            singleHeaderList.push_back(Header(line));
        header.erase(0, delimiterPosition + 2);
    }
    headerOk = singleHeaderList.size() > 0;
}

string readCompleteHeader(int clientSocket, bool& goodHeader){
    int readSize;
    string result;
    do {
        readSize = recv(clientSocket, buffer, sizeof(buffer), 0);

        // 0 -> peer closed connection, otherwise errors
        if(readSize <= 0)
            break;

        result += string(buffer, readSize);
    // check whether we have filled the buffer, faster than checking for double clrf
    } while (readSize == sizeof(buffer));

    goodHeader = (string(result, result.length() - (result.length() < 4 ? result.length() : 4)) == "\r\n\r\n");

    return result;
}

string getResponseHeader(ResponseType status){
    switch(status){
        case ResponseType::BAD_REQUEST:
            cout << "bad request" << endl;
            return "HTTP/1.0 400 Bad Request\r\n\r\n";
        case ResponseType::OK:
            cout << "ok request" << endl;
            return "HTTP/1.0 200 OK\r\n\r\n";
        case ResponseType::NOT_FOUND:
            cout << "not found request" << endl;
            return "HTTP/1.0 404 NOT FOUND\r\n\r\n";
    }
    return "rip\r\n\r\n";
}

void sendResponseHeader(ResponseType type, int socket){
    string responseHeader = getResponseHeader(type);
    if(send(socket, responseHeader.data(), responseHeader.length(), 0) < 0)
        cout << "yeah well the header wont get sent" << endl;
}

int answerRequest(FILE* file, int clientSocket,  const Header& GETHeader){
    sendResponseHeader(file == NULL ? ResponseType::NOT_FOUND : ResponseType::OK, clientSocket);

    // write file to network
    int readSize;
    do{
        //read from file
        readSize = fread(
                    buffer,
                    1,
                    sizeof(buffer),
                    file);

        // completely read file
        if(readSize <= 0)
            break;

        // try to send everything in that buffer
        if(sendall(clientSocket, buffer, &readSize) == -1)
            cout << "could only send" << readSize << endl;


    // write while we read complete pages out of the buffer
    } while(readSize == sizeof(buffer));

    // should be < 1 if the last transfer failed
    return readSize;
}

void clientConnected(NetworkError e, int clientSocket){
    if(e == NetworkError::ERROR_NO_ERROR){
        // read the full header
        bool headerOk;
        string completeHeader = readCompleteHeader(clientSocket, headerOk);

        if(!headerOk){
            sendResponseHeader(ResponseType::BAD_REQUEST, clientSocket);
            close(clientSocket);
            return;
        }

        // prepare some space for the headers
        vector<Header> headers;

        // parse the header
        parseHeaders(headers, completeHeader, headerOk);

        if(!headerOk || headers.size() <= 0 || headers.at(0).getField() != "GET"){
            sendResponseHeader(ResponseType::BAD_REQUEST, clientSocket);
            close(clientSocket);
            return;
        }

        // open the file
        FILE* file = fopen(headers.at(0).getValue().c_str(), "r");

        answerRequest(file, clientSocket, headers.at(0));

        if(file != NULL)
            fclose(file);
    }
    close(clientSocket);
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

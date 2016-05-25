#include "requestresponder.h"

using namespace std;

RequestResponder::RequestResponder(int clientSocket) : mClientSocket(clientSocket)
{
    string completeHeader;
    mHeaderList = new vector<HttpHeader>;

    if(clientSocket < 0)
        mHttpStatus = HttpStatus::BAD_SOCKET;
    else{
        if(!Network::readUntil(completeHeader, "\r\n\r\n", mBuffer, sizeof(mBuffer), mClientSocket) ||
           !HttpHeader::parseCompleteHeader(completeHeader, *mHeaderList))
            mHttpStatus = HttpStatus::BAD_REQUEST;
    }
}

RequestResponder::~RequestResponder()
{
    if(mHeaderList != nullptr)
        delete mHeaderList;
}

void RequestResponder::respond(){
    FILE* requestedFile = NULL;
    struct stat fileStatus;

    if(mHttpStatus == HttpStatus::OK){
        // somehow only works this way
        std::string sPath = mHeaderList->at(0).getValue();
        char const* path = sPath.c_str();

        if(stat(path, &fileStatus) < 0){
            mHttpStatus = HttpStatus::NOT_FOUND;
            //perror("stat failed");
        }
        else {
            requestedFile = fopen(path, "r");
            if(requestedFile == NULL)
                mHttpStatus = HttpStatus::NOT_FOUND;
        }
    }

    sendResponseHeader();

    if(mHttpStatus == HttpStatus::OK){
        int fd = fileno(requestedFile);
        if(fd >= 0)
            sendfile(mClientSocket, fd, 0, fileStatus.st_size);
        //Network::sendFile(requestedFile, mClientSocket, mBuffer, sizeof(mBuffer));
    }

    if(requestedFile != NULL)
        fclose(requestedFile);
    close(mClientSocket);
}

void RequestResponder::sendResponseHeader()
{
    string header;
    switch(mHttpStatus){
        case HttpStatus::BAD_SOCKET:
            cout << "BAD SOCKET" << endl;
            return;
        case HttpStatus::BAD_REQUEST:
            cout << "BAD REQUEST" << endl;
            header = "HTTP/1.0 400 Bad Request\r\n\r\n";
            break;
        case HttpStatus::OK:
            cout << "OK" << endl;
            header = "HTTP/1.0 200 OK\r\n\r\n";
            break;
        case HttpStatus::NOT_FOUND:
            cout << "NOT FOUND" << endl;
            header = "HTTP/1.0 404 NOT FOUND\r\n\r\n";
            break;
    }
    bool pipeBroken = false;
    if(Network::sendAll(mClientSocket, header.data(), header.length(), pipeBroken) < 0){
        cout << "header could not be sent" << endl;
        mHttpStatus = HttpStatus::BAD_SOCKET;
    }
}

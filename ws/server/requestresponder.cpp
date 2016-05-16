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
    FILE* requestedFile;
    if(mHttpStatus == HttpStatus::OK){
        requestedFile = fopen(mHeaderList->at(0).getValue().c_str(), "r");

        if(requestedFile == NULL)
            mHttpStatus = HttpStatus::NOT_FOUND;
    }

    sendResponseHeader();
    if(mHttpStatus == HttpStatus::OK){
        Network::sendFile(requestedFile, mClientSocket, mBuffer, sizeof(mBuffer));
        fclose(requestedFile);
    }

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
    if(Network::sendAll(mClientSocket, header.data(), header.length()) < 0)
        cout << "header could not be sent" << endl;
}

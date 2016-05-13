#include "requestresponder.h"

bool RequestResponder::readCompleteHeader(std::string& result){
    int readSize = 0;
    struct pollfd pollingStruct[1] = {{mClientSocket, POLLIN, 0}};
    int waitTime = 2;

    // increase wait time to up to 32 seconds
    while((waitTime *= 2) <= 32768){
        // data on socket?
        if(poll(pollingStruct, 1, waitTime) < 0)
            return false;

        // wait longer if no data is there yet
        if(!(pollingStruct[0].revents & POLLIN))
            continue;

        readSize = recv(mClientSocket, mBuffer, sizeof(mBuffer), 0);

        // client closed connection
        if(readSize == 0)
            break;

        // append to result
        result += std::string(mBuffer, readSize);

        // read the buffer full -> avoid string comp
        if(readSize == sizeof(mBuffer))
            continue;

        // if the current result ends with double crlf, the header is complete
        if(result.length() >= 4 && result.find("\r\n\r\n", result.length() - 4) != std::string::npos)
            break;
    }

    // maximum wait time reached, but the client never sent the full header
    if(result.length() < 4 || result.find("\r\n\r\n", result.length() - 4) == std::string::npos)
        return false;

    return true;
}

RequestResponder::~RequestResponder()
{
    if(mHeaders != nullptr)
        delete mHeaders;
}

RequestResponder::RequestResponder(int clientSocket) : mClientSocket(clientSocket)
{
    mResponseStatus = RespondStatus::OK;
    mHeaders = new std::vector<Header>;
    std::string completeHeader;

    if(clientSocket < 0)
        mResponseStatus = RespondStatus::INVALID_SOCKET;
    else
        if(!readCompleteHeader(completeHeader) || !Header::parseCompleteHeader(completeHeader, *mHeaders))
            mResponseStatus = RespondStatus::BAD_REQUEST;
    respond();
}

void RequestResponder::respond(){
    FILE* requestedFile;
    if(mResponseStatus == RespondStatus::OK){
        requestedFile = fopen(mHeaders->at(0).getValue().c_str(), "r");

        if(requestedFile == NULL)
            mResponseStatus = RespondStatus::NOT_FOUND;
    }

    sendResponseHeader();
    if(mResponseStatus == RespondStatus::OK){
        sendResponseData(requestedFile);
        fclose(requestedFile);
    }
    close(mClientSocket);
}

void RequestResponder::sendResponseHeader()
{
    std::string header;
    switch(mResponseStatus){
        case RespondStatus::INVALID_SOCKET:
            std::cout << "bad socket" << std::endl;
            return;
        case RespondStatus::BAD_REQUEST:
            std::cout << "bad request" << std::endl;
            header = "HTTP/1.0 400 Bad Request\r\n\r\n";
            break;
        case RespondStatus::OK:
            std::cout << "ok request" << std::endl;
            header = "HTTP/1.0 200 OK\r\n\r\n";
            break;
        case RespondStatus::NOT_FOUND:
            std::cout << "not found request" << std::endl;
            header = "HTTP/1.0 404 NOT FOUND\r\n\r\n";
            break;
    }
    if(sendAll(header.data(), header.length()) < 0)
        std::cout << "header could not be sent" << std::endl;
}

int RequestResponder::sendAll(const char *buf, int len){
    int sentBytes = 0;
    int bytesLeftToSend = len;
    int tempBytesSent;

    while(sentBytes < len) {
        // send, ignoring client-side closed connections
        tempBytesSent = send(mClientSocket, buf+sentBytes, bytesLeftToSend, MSG_NOSIGNAL);
        if (tempBytesSent == EPIPE)
            break;
        sentBytes += tempBytesSent;
        bytesLeftToSend -= tempBytesSent;
    }

    return tempBytesSent == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

void RequestResponder::sendResponseData(FILE* file)
{
    if(mResponseStatus != RespondStatus::OK)
        return;

    // write file to network
    int readSize;
    do{
        // read from file
        readSize = fread(mBuffer, 1, sizeof(mBuffer), file);

        // completely read file?
        if(readSize <= 0)
            break;

        // try to send everything in that buffer
        if(sendAll(mBuffer, readSize) == -1)
            std::cout << "could only send" << readSize << "bytes" << std::endl;


    // write while we read complete pages out of the buffer
    } while(readSize == sizeof(mBuffer));
}

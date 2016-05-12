#include "requestresponder.h"

std::string RequestResponder::readCompleteHeader(bool& goodHeader){
    int readSize;
    goodHeader = true;
    std::string result;
    do {
        readSize = recv(mClientSocket, mBuffer, sizeof(mBuffer), 0);

        // 0 -> peer closed connection, otherwise errors
        if(readSize <= 0)
            break;

        result += std::string(mBuffer, readSize);
    // check whether we have filled the buffer, faster than checking for double clrf
    } while (readSize == sizeof(mBuffer));

    if(result.length() < 4){
        goodHeader = false;
        return result;
    }

    if(std::string(result, result.length() - 4) != "\r\n\r\n")
        goodHeader = false;

    return result;
}

RequestResponder::~RequestResponder()
{
    if(mHeaders != nullptr)
        delete mHeaders;
}

RequestResponder::RequestResponder(int clientSocket) : mClientSocket(clientSocket)
{
    mResponseStatus = RespondStatus::OK;
    if(clientSocket < 0){
        mResponseStatus = RespondStatus::INVALID_SOCKET;
        return;
    }

    // read the full header
    bool headerOk;
    std::string completeHeader = readCompleteHeader(headerOk);

    std::cout << completeHeader << std::endl;

    if(!headerOk){
        mResponseStatus = RespondStatus::BAD_REQUEST;
        return;
    }

    mHeaders = new std::vector<Header>;

    // parse the header
    headerOk = Header::parseCompleteHeader(completeHeader, *mHeaders);

    if(!headerOk)
        mResponseStatus = RespondStatus::BAD_REQUEST;
}

void RequestResponder::respond(){
    sendResponseHeader();
    if(mResponseStatus == RespondStatus::OK){
        FILE* file = getFile();
        if(file == NULL)
            mResponseStatus = RespondStatus::NOT_FOUND;
        else
            sendResponseData(file);
        fclose(file);
    }
    close(mClientSocket);
    std::cout << "responded" << std::endl;
}

FILE *RequestResponder::getFile()
{
    return fopen(mHeaders->at(0).getValue().c_str(), "r");
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
        tempBytesSent = send(mClientSocket, buf+sentBytes, bytesLeftToSend, 0);
        if (tempBytesSent == -1)
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

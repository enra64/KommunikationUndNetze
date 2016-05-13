#ifndef REQUESTRESPONDER_H
#define REQUESTRESPONDER_H

#include "enums.h"
#include "header.h"

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

enum struct RespondStatus{
    INVALID_SOCKET,
    BAD_REQUEST,
    NOT_FOUND,
    OK
};

class RequestResponder
{
public:
    RequestResponder(int clientSocket);
    ~RequestResponder();
private:
    bool readCompleteHeader(std::string& result);
    int sendAll(const char* buffer, int len);
    std::vector<Header>* mHeaders = nullptr;
    void sendResponseData(FILE *file);
    RespondStatus mResponseStatus;
    void sendResponseHeader();
    int mClientSocket;
    char mBuffer[512];
    void respond();
};

#endif // REQUESTRESPONDER_H

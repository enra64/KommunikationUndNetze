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
    void respond();
private:
    char mBuffer[512];
    FILE* getFile();
    std::string readCompleteHeader(bool& goodHeader);
    int sendAll(const char* buffer, int len);
    std::vector<Header>* mHeaders = nullptr;
    RespondStatus mResponseStatus;
    void sendResponseHeader();
    void sendResponseData(FILE *file);
    int mClientSocket;
};

#endif // REQUESTRESPONDER_H

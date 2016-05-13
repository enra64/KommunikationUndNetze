#ifndef REQUESTRESPONDER_H
#define REQUESTRESPONDER_H

#include "httpheader.h"
#include "rwnetwork.h"

#include <iostream>
#include <unistd.h>
#include <thread>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

enum struct HttpStatus{
    BAD_SOCKET,
    NOT_FOUND = 404,
    BAD_REQUEST = 400,
    OK = 200
};

class RequestResponder
{
public:

    inline static void handle(int clientSocket){
        RequestResponder r(clientSocket);
        r.respond();
    }

private:
    RequestResponder(int clientSocket);
    ~RequestResponder();

    void sendResponseHeader();
    void respond();

    std::vector<HttpHeader>* mHeaderList = nullptr;
    HttpStatus mHttpStatus = HttpStatus::OK;
    int mClientSocket;
    char mBuffer[512];
};

#endif // REQUESTRESPONDER_H

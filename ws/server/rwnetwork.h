#ifndef RWNETWORK_H
#define RWNETWORK_H

#include <unistd.h>
#include <thread>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Network {
    static int sendAll(int socket, const char *buf, int len){
        int sentBytes = 0, bytesLeftToSend = len, tempBytesSent;

        while(sentBytes < len) {
            // send, ignoring client-side closed connections
            tempBytesSent = send(socket, buf+sentBytes, bytesLeftToSend, MSG_NOSIGNAL);
            if (tempBytesSent == EPIPE || tempBytesSent < 0)
                break;
            sentBytes += tempBytesSent;
            bytesLeftToSend -= tempBytesSent;
        }

        return tempBytesSent == -1 ? -1 : 0; // return -1 on failure, 0 on success
    }

    static bool sendFile(FILE* file, int socket, char* buffer, int bufferLength)    {
        int readSize = 0;

        // write while we read complete pages out of the buffer
        while(!feof(file) && ferror(file) == 0) {
            // read from file
            readSize = fread(buffer, 1, bufferLength, file);

            // try to send everything in that buffer
            if(sendAll(socket, buffer, readSize) < 0)
                return false;
        }

        // success if file error bit not set
        return ferror(file) == 0;
    }

    static bool readUntil(
            std::string& result,
            const std::string& endToken,
            char* buffer,
            size_t bufferSize,
            int socket){

        int readSize = 0, waitTime = 2;
        size_t tokenLength = endToken.length();
        struct pollfd pollingStruct[1] = {{socket, POLLIN, 0}};

        // increase wait time to up to 32 seconds
        while(waitTime <= 32768){
            // data on socket?
            if(poll(pollingStruct, 1, waitTime) < 0)
                return false;

            // wait longer if no data was available
            if(!(pollingStruct[0].revents & POLLIN)){
                waitTime *= 2;
                continue;
            }

            // try to read
            if((readSize = recv(socket, buffer, bufferSize, 0)) <= 0)
                break;

            // append to result
            result += std::string(buffer, readSize);

            // if the current result ends with double crlf, the header is complete
            if(result.length() >= tokenLength && result.find(endToken, result.length() - tokenLength) != std::string::npos)
                break;
        }

        // maximum wait time reached, but the client never sent the full header
        if(result.length() < tokenLength || result.find(endToken, result.length() - tokenLength) == std::string::npos)
            return false;

        return true;
    }
};



#endif // RWNETWORK_H

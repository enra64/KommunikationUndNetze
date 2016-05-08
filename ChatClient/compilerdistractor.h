#ifndef COMPILERDISTRACTOR_H
#define COMPILERDISTRACTOR_H

#include "netinet/in.h"

class Connector{
public:
    static int myConnect(int& fd, sockaddr* saddr, long unsigned int i){
        return connect(fd, saddr, i);
    }
};

#endif // COMPILERDISTRACTOR_H

#include <iostream>

#include <netinet/in.h>
#include <poll.h>

using namespace std;

short mPort;
unsigned long mHost
int mServerSocketHandle;

vector<int> mClients;

enum struct NetworkEvent{
    NO_ERROR
}

enum struct NetworkError{
    ERROR_NO_ERROR,
    PORT_NO_INTEGER,
    BIND_FAILED,
    LISTEN_FAILED,
    ACCEPT_FAILED,
    SOCKET_FAILED,
    POLL_FAILED
};

int closeNetwork(){
    int err = 0;

    for(int i : *mClients)
        if(close(i) < 0)
            err--;
    
    if(close(mServerSocketHandle) < 0)
        err--;

    mHost = -1;
    mPort = -1;
    mServerSocketHandle = -1;
    mClients->clear();

    return err;
}

NetworkEvent poll(){
    
}

int main(int argc, char *argv[])
{
	mPort = atoi(argv[1]);
    mHost = htonl(INADDR_ANY);

    struct sockaddr_in serverStruct;
    mServerSocketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(mServerSocketHandle < 0)
        return (int) NetworkError::SOCKET_FAILED;

    serverStruct.sin_family = AF_INET;
    serverStruct.sin_port = htons(shortport);
    serverStruct.sin_addr.s_addr = mHost;

    if(bind(
        mServerSocketHandle, 
        (struct sockaddr *) &serverStruct, 
        sizeof(serverStruct)) < 0)
        return NetworkError::BIND_FAILED;

    if(listen(mServerSocketHandle, 4) < 0)
        return NetworkError::LISTEN_FAILED;

    

    while(1){
        if(poll() != NetworkEvent::NO_ERR){
            closeNetwork();
            return NetworkError::POLL_FAILED;
        }   
    }

    cout << "Hello World!" << endl;
    return 0;
}

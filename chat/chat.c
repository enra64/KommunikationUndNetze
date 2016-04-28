/* echoclient.c */

#include <cnaiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <poll.h>

// buffer for everything
char buffer[1024];

int myPoll(int fd){
  if(fd < 0)
    return 0;
  struct pollfd pollStruct;
  pollStruct.fd = fd;
  pollStruct.events = POLLIN;
  //wait a millisecond on one fd
  poll(&pollStruct, 1, 1);
  
  if(pollStruct.revents == POLLIN)
    return 1;
  return 0;
}

void echo(int fdIn, int fdOut){
  size_t receiveLength = 0;
  // check whether input exists
  if(myPoll(fdIn)){
    // read from input
    receiveLength = 
    read(fdIn, buffer, sizeof(buffer));

    // force terminate string
    buffer[receiveLength] = 0;

    // write to output
    write(fdOut, buffer, receiveLength);
  }
}

void echoing(connection network){
  while(1){
    echo(0, network);
    echo(network, 0);
  }
}

int main(int argc, char *argv[])
{
  // valid input?
  if(argc <= 2){
    printf("usage: echoclient <computername> <appnum>\n");
    return 0;
  }

  // this is the host - ip for example. looks strange, is aber so.
  computer serverComputer;
  // application number - this directly translates to port.
  appnum applicationIdentifier;
  // network file descriptor
  connection network;

  // get server name
  serverComputer = cname_to_comp(argv[1]);

  if(serverComputer == -1){
    printf("Bad host entered, try 127.0.0.1\n");
    return 0;
  }

  // get application id
  applicationIdentifier = atoi(argv[2]);

  //try to contact the server
  network = make_contact(
                        serverComputer, 
                        applicationIdentifier);

  // check connection validity
  if(network == -1){
    printf("could not connect, starting server\n");
    // wait for connection
    network = await_contact(applicationIdentifier);
    
    //react
    if(network == -1)
      printf("no connection established\n");
    else
      printf("connection established, connection id: %i\n", network);
  }
  else{
    printf("connection established, id: %i\n", network);
  }
  
  // just echo from stdin to network and vice versa if a connection exists
  if(network != -1)
    echoing(network);
  return 0;
}

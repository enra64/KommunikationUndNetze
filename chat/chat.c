/* echoclient.c */

#include <cnaiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <poll.h>

// buffer for everything
char buffer[1024];

// if this function is called without input, it blocks
// until input is received
void echo(int fdIn, int fdOut){
  size_t receiveLength = 0;
  // read from input
  receiveLength = 
    read(fdIn, buffer, sizeof(buffer));

  // force terminate string
  buffer[receiveLength] = 0;

  // write to output
  write(fdOut, buffer, receiveLength);
}

// blocking function
void echoing(connection network){
  // polling structs
  struct pollfd pollStructs[2] = {
    {0, POLLIN, 0}, // poll stdin
    {network, POLLIN, 0}}; // poll network
  
  // echoing function
  while(1){
    // abort on polling error
	  if(poll(pollStructs, 2, -1) < 0)
	    break;
    // which fd has input?
    if(pollStructs[0].revents & POLLIN)
      echo(0, network);
    else
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

#include <cnaiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>

// buffer for everything
char buffer[1024];
// network file descriptor
connection network;

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

// handle ^C by ending contact
void sig_handler(int signal){
  if(signal == SIGINT){
    printf("\nReceived SIGINT, closing connection\n");
    end_contact(network);
    exit(0);
  }
  else
    printf("Unknown signal %i received", signal);
}

// blocking function
void echoing(){
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

  // get server name
  serverComputer = cname_to_comp(argv[1]);

  if(serverComputer == -1){
    printf("Bad host entered, try localhost\n");
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
    printf("Could not connect, starting server\n");
    // wait for connection
    network = await_contact(applicationIdentifier);
    
    //react
    if(network == -1)
      printf("No client found within timeout\n");
    else
      printf("Client connected, id: %i\n", network);
  }
  else{
    printf("Server found, id: %i\n", network);
  }
  
  // handle ^C
  if (signal(SIGINT, sig_handler) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");
  
  // just echo from stdin to network and vice versa if a connection exists
  if(network != -1)
    echoing();
  return 0;
}

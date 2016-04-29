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

// dont know what this is
computer host;
appnum port;

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

void closeNetwork(){
    if(end_contact(network) == 0)
      printf("connection successfully closed\n");
    else
      printf("connection could not be closed\n");
}

// handle ^C by ending contact
void sig_handler(int signal){
  if(signal == SIGINT){
    printf("\nReceived SIGINT, ");
    closeNetwork();
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
	  if(poll(pollStructs, 2, -1) < 0){
	    printf("Polling error, terminating. ");
	    closeNetwork();
      exit(0);
    }
    // which fd has input?
    if(pollStructs[0].revents & POLLIN)
      echo(0, network);
    else
      echo(network, 0);
  }
}

void server(){
  printf("Server started and waiting for incoming connection.\n");
  // wait for connection
  network = await_contact(port);
  
  //react
  if(network == -1)
    printf("No client found within timeout\n");
  else
    printf("Client connected, id: %i\n", network);
}


void client(){
  if(host == -1){
    printf("Bad host entered, try localhost\n");
    exit(0);
  }

  // try to contact a server
  network = make_contact(host, port);

  // check connection
  if(network == -1)
    server();
  else
    printf("Server found, id: %i\n", network);
}

int main(int argc, char *argv[])
{
  int isServer, portArgumentIndex;
  // decide on usage mode
  switch(argc){
    // no host argument provided
    case 2:
      isServer = 1;
      portArgumentIndex = 1;
      break;
    // host arg provided, try to find a server
    case 3:
      isServer = 0;
      portArgumentIndex = 2;
      break;
    // unknown argument number
    default:
      printf("Unknown Argumnt number %i\n", argc);
      printf("Client/Server usage: chat <host> <port>\n");
      printf("Server usage: chat <port>\n");
      return 0;
  }
  
  // get port
  port = atoi(argv[portArgumentIndex]);

  // run in server or client mode?
  if(isServer)
    server();
  else{
    // get host
    host = cname_to_comp(argv[1]);
    client();
  }
  
  
  // handle ^C
  if (signal(SIGINT, sig_handler) == SIG_ERR)
    printf("\nCan't catch SIGINT\n");
  
  // network ok, start chat method
  if(network != -1)
    echoing();
  return 0;
}

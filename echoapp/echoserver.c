/* echoserver.c */

#include <cnaiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
  if(argc != 2){
    printf("usage: echoserver <appnum>\n");
    return 0;
  }

  // get application id
  appnum applicationIdentifier = 
      atoi(argv[1]);

  int socket = 
      get_server_socket(applicationIdentifier);

  printf("Using socket %i\n", socket);

  printf("Waiting for connection...\n");

  // wait for connection
  connection clientConnection = 
      await_contact(applicationIdentifier);

  //react
  if(clientConnection == -1)
    printf("no connection established\n");
  else
    printf("connection established, connection id: %i\n", clientConnection);

  char receivingBuffer[1024];
  int receiveLength = 0;
  while(1){
    // check network for input
    while((receiveLength = read(clientConnection, receivingBuffer, sizeof(receivingBuffer) - 1)) > 0){
      // force string end      
      receivingBuffer[receiveLength] = 0;

      // fputs(receivingBuffer, stdout)
      write(clientConnection, receivingBuffer, receiveLength);
    }
  }
	return 0;
}

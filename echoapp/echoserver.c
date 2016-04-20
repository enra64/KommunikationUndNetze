/* echoserver.c */

#include <cnaiapi.h>
#include <stdio.h>
#include <stdlib.h>

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
	return 0;
}

/* echoclient.c */

#include <cnaiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

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
  // probably something from netinet in a new struct?
  connection serverConnection;

  // get server name
  serverComputer = cname_to_comp(argv[1]);

  if(serverComputer == -1){
    printf("Bad host entered, try 127.0.0.1\n");
    return 0;
  }

  // get application id
  applicationIdentifier = atoi(argv[2]);

  //try to contact the server
  serverConnection = make_contact(
                        serverComputer, 
                        applicationIdentifier);

  // check connection validity
  if(serverConnection == -1){
    printf("could not connect\n");
    return 0;
  }
  else
    printf("connection established, id: %i\n", serverConnection);

  // used for input from stdin
  char userBuffer[1024];
  size_t userMsgLength;
  
  // used for input from network
  char serverBuffer[1024];  
  int serverMsgLength;
  
  while(1){
    // check stdin for new input
    if((userMsgLength = read(0, userBuffer, 1024)) > 0){
      // write to connection
      write(serverConnection, userBuffer, userMsgLength);
    }

    // check network for new output
    if((serverMsgLength = read(serverConnection, serverBuffer, sizeof(serverBuffer) - 1)) > 0){
      printf("Echo from server:\n");

      // welcome to c - end string
      serverBuffer[serverMsgLength] = 0;

      // write server input to stdout
      fputs(serverBuffer, stdout);
    }
  }
  // write(serverConnection, "test", 4);
	return 0;
}

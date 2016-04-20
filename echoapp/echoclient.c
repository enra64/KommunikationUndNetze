/* echoclient.c */

#include <cnaiapi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if(argc != 3){
    printf("usage: echoclient <computername> <appnum>\n");
    return 0;
  }

  // get server name
  computer serverComputer = 
      cname_to_comp(argv[1]);

  if(serverComputer == -1){
    printf("Bad host entered, try 127.0.0.1");
    return 0;
  }

  // get application id
  appnum applicationIdentifier = 
      atoi(argv[2]);

  //try to contact the server
  connection serverConnection = 
      make_contact(serverComputer, applicationIdentifier);

  if(serverConnection == -1)
    printf("no connection accomplished\n");
  else
    printf("connection established, id: %i\n", serverConnection);

  
	return 0;
}

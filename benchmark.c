/*--------------------------------------------------------------------------------------------------
  This is just a test client, which let's you directly type to your console application.
-------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comclient.h"

#define NUM_LOOPS 1000000

int main(
    int argc,
    char **argv)
{
    char *response;
    char *sessionID = "benchmark";
    int i;

    if(argc != 2) {
        printf("Usage: %s fileSocketPath\n", argv[0]);
        return 1;
    }
    response = coStartClient(argv[1], sessionID);
    for(i = 0; i < NUM_LOOPS; i++) {
	if(i & 1) {
	    coSendMessage("login admin donttell");
	    response = coReadMessage();
	} else {
	    coSendMessage("logout");
	    response = coReadMessage();
	}
    }
    coStopClient();
    return 0;
}


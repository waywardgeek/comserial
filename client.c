/*--------------------------------------------------------------------------------------------------
  This is just a test client, which let's you directly type to your console application.
-------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comclient.h"

int main(
    int argc,
    char **argv)
{
    char line[100];
    char *response;

    if(argc != 3) {
	printf("Usage: %s fileSocketPath sessionId\n", argv[0]);
	return 1;
    }
    coStartClient(argv[1], argv[2]);
    while(fgets(line, 100, stdin) && strcmp(line, "quit")) {
        coSendMessage("%s", line);
        response = coReadMessage();
        printf("%s", response);
    }
    coStopClient();
    return 0;
}


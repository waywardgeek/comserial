/*--------------------------------------------------------------------------------------------------
  This is just a test client, which let's you directly type to your console application.
-------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgiutil.h"
#include "comclient.h"

int main(
    int argc,
    char **argv)
{
    char line[100];
    char *response;
    char *sessionID = cgiGenerateRandomID(20);

    if(argc != 2) {
        printf("Usage: %s fileSocketPath\n", argv[0]);
        return 1;
    }
    response = coStartClient(argv[1], sessionID);
    printf("%s", response);
    while(fgets(line, 100, stdin) && strcmp(line, "quit")) {
        coSendMessage("%s", line);
        response = coReadMessage();
        printf("%s", response);
    }
    coStopClient();
    return 0;
}


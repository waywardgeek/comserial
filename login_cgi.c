/*--------------------------------------------------------------------------------------------------
  Login script.
-------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "cgiutil.h"
#include "comclient.h"

int main(void)
{
    char *input, *session, *userName, *password, *response;
    FILE *f;

    cgiEnableDebug("/tmp/log");
    cgiPrintf("Content-Type:text/html;charset=iso-8859-1\n");
    session = cgiReadSessionId(); /* This must come here so that we can set session cookie */
    cgiPrintf("\n"); /* This ends the "header" of the response */
    input = cgiReadInput();
    if(input == NULL) {
        cgiPrintf("<P>Error in invocation - wrong FORM probably.");
    } else {
        userName = cgiReadInputVar(input, "user");
        password = cgiReadInputVar(input, "password");
        cgiPrintf("starting client with sessionId=%s<br>", session);
        coStartClient("/tmp/test_socket", session);
        cgiPrintf("sending login %s %s<br>", userName, password);
        coSendMessage("login %s %s\n", userName, password);
        cgiPrintf("reading response from server<br>");
        response = coReadMessage();
        cgiPrintf("response = %s", response);
        coStopClient();
    }
    return 0;
}


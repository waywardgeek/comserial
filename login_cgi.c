/*--------------------------------------------------------------------------------------------------
  Login script.
-------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cgiutil.h"

int main(void)
{
    char *input, *session, *userName, *password;
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
        cgiPrintf("User = %s, password = %s", userName, password);
    }
    return 0;
}


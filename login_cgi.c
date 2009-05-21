/*=================================================================================================
  Login script.
=================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cgi_util.h"

int main(void)
{
    char *input, *session, *userName, *password;
    FILE *f;

    enableDebug("/tmp/log");
    print("Content-Type:text/html;charset=iso-8859-1\n");
    session = readSessionId(); /* This must come here so that we can set session cookie */
    print("\n"); /* This ends the "header" of the response */
    input = readInput();
    if(input == NULL) {
        print("<P>Error in invocation - wrong FORM probably.");
    } else {
	userName = readInputVar(input, "user");
	password = readInputVar(input, "password");
	print("User = %s, password = %s", userName, password);
    }
    return 0;
}


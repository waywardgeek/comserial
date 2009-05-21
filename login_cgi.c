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
    session = readCookie("sessionId");
    if(session == NULL) {
        session = generateSessionId();
        print("Set-Cookie: sessionId=%s\n", session);
    }
    print("\n");
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


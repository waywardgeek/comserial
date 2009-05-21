/*=================================================================================================
  This example CGI script maintains a session ID, and writes user input to a file in /tmp/data.txt.
=================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cgi_util.h"

int main(void)
{
    char *input, *session;
    FILE *f;

    enableDebug("/tmp/log");
    print("Content-Type:text/html;charset=iso-8859-1\n");
    session = readCookie("sessionId");
    if(session == NULL) {
        session = generateSessionId();
        print("Set-Cookie: sessionId=%s\n", session);
    }
    print("\n");
    print("<TITLE>Response</TITLE>\n");
    input = readInput();
    if(input == NULL) {
        print("<P>Error in invocation - wrong FORM probably.");
    } else {
        f = fopen(DATAFILE, "a");
        if(f == NULL) {
            print("<P>Sorry, cannot store your data.");
        } else {
            fprintf(f, "%s:%s", session, input);
	}
        fclose(f);
        print("<P>Thank you! Your contribution has been stored.");
    }
    return 0;
}


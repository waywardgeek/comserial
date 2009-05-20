/*=================================================================================================
  This example CGI script maintains a session ID, and writes user input to a file in /tmp/data.txt.
=================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cgi_util.h"

#define DATAFILE "/tmp/data.txt"
#define MAXLEN 100000000
#define EXTRA 5

int main(void)
{
    char *lenstr;
    char *input, *data;
    long len;
    char *session;
    FILE *f;

    enableDebug("/tmp/log");
    session = readCookie("sessionId");
    print("Content-Type:text/html;charset=iso-8859-1\n");
    if(session == NULL) {
        session = generateSessionId();
        print("Set-Cookie: sessionId=%s\n", session);
    }
    print("\n");
    print("<TITLE>Response</TITLE>\n");
    lenstr = getenv("CONTENT_LENGTH");
    if(lenstr == NULL || sscanf(lenstr,"%ld",&len)!=1 || len > MAXLEN) {
        print("<P>Error in invocation - wrong FORM probably.");
    } else {
	input = (char *)calloc(len + 1, sizeof(char));
        fgets(input, len + 1, stdin);
	data = (char *)calloc(len + 3, sizeof(char));
        unencode(input + EXTRA, input + len, data);
        f = fopen(DATAFILE, "a");
        if(f == NULL) {
            print("<P>Sorry, cannot store your data.");
        } else {
            fprintf(f, "%s:%s", session, data);
	}
        fclose(f);
        print("<P>Thank you! Your contribution has been stored.");
    }
    return 0;
}


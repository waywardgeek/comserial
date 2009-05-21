/*=================================================================================================
  This example CGI script maintains a session ID, and writes user input to a file in /tmp/data.txt.
=================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "cgi_util.h"

static char *varBuf = NULL, *valueBuf = NULL;
static long varBufSize = 42, valueBufSize = 42;
static FILE *debugFile = NULL;

/*=================================================================================================
  Enable debug logging to the file.
=================================================================================================*/
void enableDebug(
    char *logFile)
{
    debugFile = fopen(logFile, "w");
}

/*=================================================================================================
  A print function that can optionally log to a log file while printing.
=================================================================================================*/
void print(
    char *format,
    ...)
{
    va_list ap;
    char buffer[256];

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    printf("%s", buffer);
    if(debugFile != NULL) {
        fprintf(debugFile, "%s", buffer);
        fflush(debugFile);
    }
}

/*=================================================================================================
  Just unencode the string.  Do it in place.
=================================================================================================*/
void unencode(
    char *string)
{
    char *p = string;
    int code;

    while(*string != '\0') {
        if(*string == '+') {
            *p = ' ';
        } else if(*string == '%') {
            if(sscanf(string + 1, "%2x", &code) != 1) {
		code = '?';
	    }
            *p = code;
            string += 2;
	} else {
            *p = *string;
	}
        string++;
	p++;
    }
    *p = '\0';
}

/*=================================================================================================
  Read the next variable name and value in the cookie string, and return a pointer to the next
  variable/value pair in the cookie string.
=================================================================================================*/
static char *readNextVariableValueCombo(
    char *string,
    char separator)
{
    char c;
    int varPos = 0, valuePos = 0;

    while((c = *string++) != '=' && c != '\0') {
	if(varPos + 1 == varBufSize) {
	    varBufSize <<= 1;
	    varBuf = (char *)realloc(varBuf, varBufSize*sizeof(char));
	}
	varBuf[varPos++] = c;
    }
    varBuf[varPos] = '\0';
    while((c = *string++) != separator && c != '\0') {
	if(valuePos + 1 == valueBufSize) {
	    valueBufSize <<= 1;
	    valueBuf = (char *)realloc(valueBuf, valueBufSize*sizeof(char));
	}
	valueBuf[valuePos++] = c;
    }
    valueBuf[valuePos] = '\0';
    while(*string == ' ') {
	string++;
    }
    unencode(varBuf);
    unencode(valueBuf);
    return string;
}

/*=================================================================================================
  Read a cookie variable.  Return NULL if not set.
=================================================================================================*/
char *readCookie(
    char *varName)
{
    char *cookies = getenv("HTTP_COOKIE");
    char *retVal;

    if(varBuf == NULL) {
	varBuf = (char *)calloc(varBufSize, sizeof(char));
	valueBuf = (char *)calloc(valueBufSize, sizeof(char));
    }
    if(cookies == NULL) {
	return NULL;
    }
    while(*cookies != '\0') {
        cookies = readNextVariableValueCombo(cookies, ';');
        if(!strcmp(varName, varBuf)) {
	    retVal = (char *)calloc(strlen(valueBuf) + 1, sizeof(char));
	    strcpy(retVal, valueBuf);
            return retVal;
        }
    }
    return NULL;
}

/*=================================================================================================
  Generate a random session ID.
=================================================================================================*/
char *generateSessionId(void)
{
    FILE *randFile = fopen("/dev/urandom", "r");
    static char randChars[21];
    char *string = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-";
    int i;

    for(i = 0; i < 20; i++) {
        randChars[i] = string[getc(randFile) & 0x3f];
    }
    randChars[20] = '\0';
    fclose(randFile);
    return randChars;
}

/*=================================================================================================
  Read input.
=================================================================================================*/
char *readInput(void)
{
    char *lenstr = getenv("CONTENT_LENGTH");
    char *input;
    long len;

    if(lenstr == NULL) {
	print("CONTENT_LENGTH not set");
	return NULL;
    }
    if(sscanf(lenstr,"%ld",&len) != 1 || len > INPUT_MAXLEN) {
	print("Invalid input length");
	return NULL;
    }
    input = (char *)calloc(len + 1, sizeof(char));
    fgets(input, len + 1, stdin);
    return input;
}

/*=================================================================================================
  Read an input variable from the input string.
=================================================================================================*/
char *readInputVar(
    char *input,
    char *varName)
{
    char *retVal;

    if(varBuf == NULL) {
	varBuf = (char *)calloc(varBufSize, sizeof(char));
	valueBuf = (char *)calloc(valueBufSize, sizeof(char));
    }
    while(*input != '\0') {
        input = readNextVariableValueCombo(input, '&');
        if(!strcmp(varName, varBuf)) {
	    retVal = (char *)calloc(strlen(valueBuf) + 1, sizeof(char));
	    strcpy(retVal, valueBuf);
            return retVal;
        }
    }
    return NULL;
}

/*=================================================================================================
  Read the sessionId cookie, and if not set, set it.  Return the sessionId.
=================================================================================================*/
char *readSessionId(void)
{
    char *session = readCookie("sessionId");

    if(session == NULL) {
        session = generateSessionId();
        print("Set-Cookie: sessionId=%s\n", session);
    }
    return session;
}

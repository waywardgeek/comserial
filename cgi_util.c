/*=================================================================================================
  This example CGI script maintains a session ID, and writes user input to a file in /tmp/data.txt.
=================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
  Just unencode the string.
=================================================================================================*/
void unencode(
    char *src,
    char *last,
    char *dest)
{
    int code;

    while(src != last) {
        if(*src == '+') {
            *dest = ' ';
        } else if(*src == '%') {
            if(sscanf(src+1, "%2x", &code) != 1) code = '?';
            *dest = code;
            src += 2;
	} else {
            *dest = *src;
	}
        src++;
	dest++;
    }
    *dest = '\n';
    *++dest = '\0';
}

/*=================================================================================================
  Read the next variable name and value in the cookie string, and return a pointer to the next
  variable/value pair in the cookie string.
=================================================================================================*/
static char *readNextCookie(
    char *cookies)
{
    char c;
    int varPos = 0, valuePos = 0;

    while((c = *cookies++) != '=' && c != '\0') {
	if(varPos + 1 == varBufSize) {
	    varBufSize <<= 1;
	    varBuf = (char *)realloc(varBuf, varBufSize*sizeof(char));
	}
	varBuf[varPos++] = c;
    }
    varBuf[varPos] = '\0';
    while((c = *cookies++) != ';' && c != '\0') {
	if(valuePos + 1 == valueBufSize) {
	    valueBufSize <<= 1;
	    valueBuf = (char *)realloc(valueBuf, valueBufSize*sizeof(char));
	}
	valueBuf[valuePos++] = c;
    }
    valueBuf[valuePos] = '\0';
    while(*cookies == ' ') {
	cookies++;
    }
    return cookies;
}

/*=================================================================================================
  Read a cookie variable.  Return NULL if not set.
=================================================================================================*/
char *readCookie(
    char *varName)
{
    char *cookies = getenv("HTTP_COOKIE");

    if(varBuf == NULL) {
	varBuf = (char *)calloc(varBufSize, sizeof(char));
	valueBuf = (char *)calloc(valueBufSize, sizeof(char));
    }
    if(cookies == NULL) {
	return NULL;
    }
    while(*cookies != '\0') {
        cookies = readNextCookie(cookies);
        if(!strcmp(varName, varBuf)) {
            return valueBuf;
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

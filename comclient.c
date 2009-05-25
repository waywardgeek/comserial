//
// Client.cpp
//
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "comclient.h"

static char *coSessionId;
static int coSockfd;
static char *coMessage;
static int coMessageSize;

/*--------------------------------------------------------------------------------------------------
  Start the client.  Connect to the server's file socket, and return non-zero on success.
--------------------------------------------------------------------------------------------------*/
void coStartClient(
    char *fileSocketPath,
    char *sessionId)
{
    struct sockaddr_un address;
    int len;
    int result;

    coSockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, fileSocketPath);
    len = strlen(fileSocketPath);
    result = connect(coSockfd, (struct sockaddr*)&address, len);
    if(result == -1) {
        perror("failed to open socket");
        exit(1);
    }
    coSessionId = calloc(strlen(sessionId) + 1, sizeof(char));
    strcpy(coSessionId, sessionId);
    write(coSockfd, coSessionId, strlen(coSessionId));
    write(coSockfd, "\0", 1);
    coMessageSize = 42;
    coMessage = calloc(coMessageSize, sizeof(char));
}

/*--------------------------------------------------------------------------------------------------
  Stop the client.  Disconect from the server's file socket.
--------------------------------------------------------------------------------------------------*/
void coStopClient(void)
{
    close(coSockfd);
}

/*--------------------------------------------------------------------------------------------------
  Send a message to the server.
--------------------------------------------------------------------------------------------------*/
void coSendMessage(
    char *format,
    ...)
{
    va_list ap;
    char buffer[CO_MAX_MESSAGE_LENGTH];
    int length;

    va_start(ap, format);
    length = vsnprintf(buffer, CO_MAX_MESSAGE_LENGTH, format, ap);
    va_end(ap);
    write(coSockfd, buffer, length + 1); /* Include terminating '\0' */
}

/*--------------------------------------------------------------------------------------------------
  Send a message to the server.
--------------------------------------------------------------------------------------------------*/
char *coReadMessage(void)
{
    char c;
    int messagePos = 0;

    while(read(coSockfd, &c, 1) == 1) {
        if(messagePos + 1 == coMessageSize) {
            coMessageSize <<= 1;
            coMessage = (char *)realloc(coMessage, coMessageSize*sizeof(char));
        }
        coMessage[messagePos++] = c;
    }
    coMessage[messagePos] = '\0';
    return coMessage;
}

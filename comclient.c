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
  Start the client.  Connect to the server's file socket, and return the greeting message.
  return NULL on failure.
--------------------------------------------------------------------------------------------------*/
char *coStartClient(
    char *fileSocketPath,
    char *sessionId)
{
    struct sockaddr_un address;
    int len;
    int result;
    char response[3];

    coSockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, fileSocketPath);
    len = strlen(fileSocketPath) + 2;
    result = connect(coSockfd, (struct sockaddr*)&address, len);
    if(result == -1) {
        perror("failed to open socket");
        return NULL;
    }
    coSessionId = calloc(strlen(sessionId) + 1, sizeof(char));
    strcpy(coSessionId, sessionId);
    len = write(coSockfd, coSessionId, strlen(coSessionId) + 1);
    fsync(coSockfd);
    coMessageSize = 42;
    coMessage = calloc(coMessageSize, sizeof(char));
    return coReadMessage();
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
    length = write(coSockfd, buffer, length + 1); /* Include terminating '\0' */
    fsync(coSockfd);
}

/*--------------------------------------------------------------------------------------------------
  Send a message to the server.
--------------------------------------------------------------------------------------------------*/
char *coReadMessage(void)
{
    char c;
    int messagePos = 0;

    do {
        if(read(coSockfd, &c, 1) != 1) {
            perror("Cannot read from server");
            exit(1);
        }
        if(messagePos + 1 == coMessageSize) {
            coMessageSize <<= 1;
            coMessage = (char *)realloc(coMessage, coMessageSize*sizeof(char));
        }
        coMessage[messagePos++] = c;
    } while(c != '\0');
    return coMessage;
}

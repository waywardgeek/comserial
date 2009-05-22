//
// Client.cpp
//
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "comclient.h"

static char *coSessionId;
static int coSockfd;

/*=================================================================================================
  Start the client.  Connect to the server's file socket, and return non-zero on success.
=================================================================================================*/
int coStartClient(
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
    return 1;
}

/*=================================================================================================
  Stop the client.  Disconect from the server's file socket.
=================================================================================================*/
void coStopClient(void)
{
    close(coSockfd);
}

/*=================================================================================================
  Send a message to the server.
=================================================================================================*/
void coSendMessage(
    char *format,
    ...)
{
    va_list ap;
    char buffer[CO_MAX_STRING_LENGTH];
    int length;

    va_start(ap, format);
    length = vsnprintf(buffer, CO_MAX_STRING_LENGTH, format, ap);
    va_end(ap);
    write(coSockfd, coSessionId, strlen(coSessionId));
    write(coSockfd, " ", 1);
    write(coSockfd, buffer, length + 1); /* Include terminating '\0' */
}

/*=================================================================================================
  Send a message to the server.
=================================================================================================*/
char *coReadMessage(void);

int main()
{
    int         sockfd;
    int         len;
    struct sockaddr_un address;
    int         result;
    char        ch       = 'A';

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, "server_socket");
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr*)&address, len);

    if(result == -1) {
        perror("oops:  client1");
        exit(1);
    }

    write(sockfd, &ch, 1);
    read(sockfd, &ch, 1);
    printf("char from server = %c\n", ch);
    close(sockfd);
    exit(0);
}

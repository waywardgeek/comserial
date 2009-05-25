#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "comserver.h"
#include "comclient.h" /* for max message length */

struct coClientStruct;
typedef unsigned int uint;
typedef struct coClientStruct *coClient;

struct coClientStruct {
    int sockfd;
    char *message;
    uint messageSize, messageLength;
    uint messagePos;
    char *sessionId;
};

static coClient *coClientTable;
static uint coClientTableSize = 16;
static int coServerSockfd;
static char *coSocketPath;
static coClient coCurrentClient;
static int coServerStarted = 0;
static coEndSessionProc coEndSession = NULL;

/*--------------------------------------------------------------------------------------------------
  Set a file descriptor to be non-blocking.  This allows accept to return right away, without
  waiting for a connection to be made.
--------------------------------------------------------------------------------------------------*/
static void setSocketNonBlocking(
    int socket,
    int value)
{
    int flags = fcntl(socket, F_GETFL);

    if(value) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    if(fcntl(socket, F_SETFL, flags) < 0) {
        perror("Trouble setting socket non-block mode");
        exit(1);
    }
}

/*--------------------------------------------------------------------------------------------------
  Start the server, and open the socket for listening.  Return non-zero on success.
--------------------------------------------------------------------------------------------------*/
void coStartServer(
    char *fileSocketPath)
{
    int serverLen;
    struct sockaddr_un serverAddress;

    unlink(fileSocketPath);
    coServerSockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    setSocketNonBlocking(coServerSockfd, 1);
    serverAddress.sun_family = AF_UNIX;
    strncpy(serverAddress.sun_path, fileSocketPath, 108);
    serverAddress.sun_path[108 - 1] = '\0';
    serverLen = strlen(serverAddress.sun_path) + 2;
    if(bind(coServerSockfd, (struct sockaddr*)&serverAddress, serverLen)) {
        perror("failed to bind socket");
        exit(1);
    }
    listen(coServerSockfd, 5);
    coSocketPath = calloc(strlen(fileSocketPath) + 1, sizeof(char));
    strcpy(coSocketPath, fileSocketPath);
    coClientTable = (coClient *)calloc(coClientTableSize, sizeof(coClient));
    coServerStarted = 1;
}

/*--------------------------------------------------------------------------------------------------
  Stop the server.
--------------------------------------------------------------------------------------------------*/
void coStopServer(void)
{
    unlink(coSocketPath);
    free(coSocketPath);
    free(coClientTable);
    close(coServerSockfd);
}

/*--------------------------------------------------------------------------------------------------
  Accept pending connections on our listening port.  Return it's socket file descriptor.
--------------------------------------------------------------------------------------------------*/
static int acceptConnection(void)
{
    struct sockaddr_un clientAddress;
    int length = sizeof(clientAddress);
    int clientSockfd = accept(coServerSockfd, (struct sockaddr*)&clientAddress, &length);

    if(clientSockfd < 0)  {
        perror("Failed to accept socket");
        exit(1);
    }
    setSocketNonBlocking(clientSockfd, 1);
    return clientSockfd;
}

/*--------------------------------------------------------------------------------------------------
  Accept pending connections on our listening port.  Create a new client object for it.
--------------------------------------------------------------------------------------------------*/
static void acceptNewClient(void)
{
    int sockfd = acceptConnection();
    coClient client = (coClient)calloc(1, sizeof(struct coClientStruct));

    client->sockfd = sockfd;
    client->messageSize = 42;
    client->message = (char *)calloc(client->messageSize, sizeof(char));
    if(sockfd >= coClientTableSize) {
        coClientTableSize = sockfd + (sockfd >> 1);
        coClientTable = (coClient *)realloc(coClientTable, coClientTableSize*sizeof(coClient));
    }
    coClientTable[sockfd] = client;
}

/*--------------------------------------------------------------------------------------------------
  See if any message is complete, and if so, and return the client with a complete message.
--------------------------------------------------------------------------------------------------*/
static coClient readMessage(
    fd_set readSockets)
{
    coClient client, readyClient = NULL;
    int xClient;
    char buf[CO_MAX_MESSAGE_LENGTH];
    ssize_t length;

    for(xClient = 0; xClient < coClientTableSize; xClient++) {
        client = coClientTable[xClient];
        if(client != NULL) {
#ifdef DEBUG
            printf("Checking client %u:%s\n", xClient, client->sessionId);
#endif
            if(FD_ISSET(xClient, &readSockets)) {
                length = read(xClient, buf, CO_MAX_MESSAGE_LENGTH);
#ifdef DEBUG
                printf("Just read %d bytes\n", length);
#endif
                if(length <= 0) {
                    if(errno != EAGAIN) {
                        /* I don't know why sometimes sockets that have ID_ISSET true can't be read yet */
                        /* Close client */
                        if(coEndSession != NULL) {
                            coEndSession(client->sessionId);
                        }
                        close(xClient);
                        free(client->message);
                        free(client);
                        coClientTable[xClient] = NULL;
                    }
                } else if(client->sessionId == NULL) {
                    /* Must be initial sessionId message */
                    client->sessionId = (char *)calloc(length, sizeof(char));
                    strcpy(client->sessionId, buf);
#ifdef DEBUG
                    printf("Got sessionId %s, length %u\n", buf, length);
#endif
		    write(coCurrentClient->sockfd, "OK", 3);
                } else {
#ifdef DEBUG
                    printf("Read message of length %d: '%s'\n", (int)length, buf);
#endif
                    if(length + client->messageLength >= client->messageSize) {
                        client->messageSize = length + client->messageLength + (client->messageSize >> 1);
                        client->message = (char *)realloc(client->message, client->messageSize*sizeof(char));
                    }
                    strncpy(client->message + client->messageLength, buf, length);
                    client->messageLength += length;
                    if(client->message[client->messageLength - 1] == '\0') {
                        /* Completed message */
                        readyClient = client;
                        client->messagePos = 0;
                    }
                }
            }
        }
    }
    return readyClient;
}

/*--------------------------------------------------------------------------------------------------
  This command waits for a completed message from a client, and then returns the sessionId of
  that client.  Any calls to coPrintf after this call will direct the text to that client.
--------------------------------------------------------------------------------------------------*/
char *coStartResponse(void)
{
    coClient client;
    fd_set readSockets;
    struct timeval tv;
    int maxSocket;
    int numActiveSockets;
    int xClient;

    if(!coServerStarted) {
        return "sessionId";
    }
    do {
        maxSocket = coServerSockfd;
        FD_ZERO(&readSockets);
        for(xClient = 0; xClient < coClientTableSize; xClient++) {
            client = coClientTable[xClient];
            if(client != NULL) {
                if(client->sockfd > maxSocket) {
                    maxSocket = client->sockfd;
                }
                FD_SET(client->sockfd, &readSockets);
            }
        }
        FD_SET(coServerSockfd, &readSockets);
        /* Wait up to 2 seconds. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        numActiveSockets = select(maxSocket + 1, &readSockets, NULL, NULL, &tv);
        if(numActiveSockets < 0) {
            perror("Select error");
            exit(1);
        }
        if(FD_ISSET(coServerSockfd, &readSockets)) {
            acceptNewClient();
        }
        client = readMessage(readSockets);
    } while(client == NULL);
    coCurrentClient = client;
    return client->sessionId;
}

/*--------------------------------------------------------------------------------------------------
  Read a character from the active client's message buffer.
--------------------------------------------------------------------------------------------------*/
int coGetc(void)
{
    if(!coServerStarted) {
        return getchar();
    }
    if(coCurrentClient->messagePos == coCurrentClient->messageLength) {
        return EOF;
    }
#ifdef DEBUG
    printf("Read char '%c' 0x%x\n", coCurrentClient->message[coCurrentClient->messagePos],
            coCurrentClient->message[coCurrentClient->messagePos]);
#endif
    return coCurrentClient->message[(coCurrentClient->messagePos)++];
}

/*--------------------------------------------------------------------------------------------------
  Write a message to the client.  If coStartServer was never called, just call coPrintf.
--------------------------------------------------------------------------------------------------*/
int coPrintf(
    char *format,
    ...)
{
    va_list ap;
    char buffer[CO_MAX_MESSAGE_LENGTH];
    int length;

    va_start(ap, format);
    length = vsnprintf(buffer, CO_MAX_MESSAGE_LENGTH, format, ap);
    va_end(ap);
    //temp - fix this to use local write buffers that are driven with select
    // this will fail if the client is not ready for a write
    if(coServerStarted) {
        write(coCurrentClient->sockfd, buffer, length);
    } else {
        printf("%s", buffer);
    }
    return length;
}

/*--------------------------------------------------------------------------------------------------
  Just send the '\0' to terminate the message.
--------------------------------------------------------------------------------------------------*/
void coCompleteResponse(void)
{
    if(coServerStarted) {
        write(coCurrentClient->sockfd, "", 1);
	fsync(coCurrentClient->sockfd);
        coCurrentClient->messageLength = 0;
        coCurrentClient->messagePos = 0;
        coCurrentClient = NULL;
    }
}

/*--------------------------------------------------------------------------------------------------
  Just set the coEndSession callback.
--------------------------------------------------------------------------------------------------*/
void coSetEndSessionCallback(
    coEndSessionProc endSession)
{
    coEndSession = endSession;
}

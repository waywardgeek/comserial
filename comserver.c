#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include "comserver.h"
#include "comclient.h" /* for max message length */

struct coClientStruct;
typedef unsigned int uint;
typedef struct coClientStruct *client;

struct coClientStruct {
    int sockfd;
    char *message;
    uint messageSize, messageLength;
    uint messagePos;
    char *sessionId;
};

static coClient coFirstClient = NULL, coLastClient = NULL;
static coClient *coClientTable;
static uint coClientTableSize = 16;
static uint coNumClients = 0;
static int coServerSockfd;
static char *coSocketPath;
static coClient coCurrentClient;

/*--------------------------------------------------------------------------------------------------
  Set a file descriptor to be non-blocking.  This allows accept to return right away, without
  waiting for a connection to be made.
--------------------------------------------------------------------------------------------------*/
static void setSocketNonBlocking(
    int socket,
    int value)
{
    UTINT flags = fcntl(socket, F_GETFL);

    if(value) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    if(fcntl(socket, F_SETFL, flags) < 0) {
        perror("Trouble setting socket non-block mode: error #%d: %s", errno, strerror(errno));
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
    strcpy(serverAddress.sun_path, fileSocketPath);
    serverLen = strlen(serverAddress) + 1;
    if(bind(coServerSockfd, (struct sockaddr*)&serverAddress, serverLen)) {
        perror("failed to bind socket");
        exit(1);
    }
    listen(coServerSockfd, 5);
    coSocketPath = calloc(strlen(fileSocketPath) + 1, sizeof(char));
    strcpy(soSocketPath, fileSocketPath);
    coClientTable = (coClient *)calloc(coClientTableSize, sizeof(coClient));
}

/*--------------------------------------------------------------------------------------------------
  Stop the server.
--------------------------------------------------------------------------------------------------*/
void coStopServer(void)
{
    unlink(soSocketPath);
    free(soSocketPath);
    free(coClientTable);
    close(soServerSockfd);
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
    setSocketNonBlocking(clientSockfd, true);
    return clientSockfd;
}

/*--------------------------------------------------------------------------------------------------
  Accept pending connections on our listening port.  Create a new client object for it.
--------------------------------------------------------------------------------------------------*/
static void acceptNewClient(void)
{
    int sockfd = acceptConnection(void);
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
  Read some bytes from the socket, and incrementally build messages with them.  When we have
  completed messages, process them with bsProcessMessageCallback.
--------------------------------------------------------------------------------------------------*/
static void readMessage(
    bsConnection connection)
{
    ssize_t xByte;

    if(length <= 0) {
        if(errno == EAGAIN) {
            /* I don't know why sometimes sockets that have ID_ISSET true can't be read yet */
            return;
        }
        if(length == 0) {
            bsConnectionClose(connection, utSprintf("Connection closed to IP %s:%u",
                bsConnectionGetIp(connection), bsConnectionGetPort(connection)));
        } else {
            bsLogLibError("Failed to read from socket");
            bsConnectionClose(connection, "");
        }
        return;
    }
    for(xByte = 0; xByte < length; xByte++) {
        if(!pushByteIntoConnection(connection, buf[xByte])) {
            return;
        }
    }
}

/*--------------------------------------------------------------------------------------------------
  See if any message is complete, and if so, and return it.
--------------------------------------------------------------------------------------------------*/
static char *readMessage(
    fd_set readSockets)
{
    coClient client, readyClient = NULL;
    int xClient;
    char buf[CO_MAX_MESSAGE_LENGTH];
    ssize_t length;

    for(xClient = 0; xClient < coClientTableSize; xClient++) {
        client = coClientTable(xClient);
        if(client != NULL) {
            if(FD_ISSET(xClient, &readSockets)) {
                length = read(xClient, buf, CO_MAX_MESSAGE_LENGTH);
                if(length <= 0) {
                    /* Close client */
                    close(xClient);
                    free(client);
                    coClientTable[xClient] = NULL;
                    // temp: need to indicate to server that client has gone away
                } else {
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

    do {
        maxSocket = coServerSockfd;
        FD_ZERO(&readSockets);
        for(client = coFirstClient; client != NUll; client = client->next) {
            maxSocket = utMax(client->sockfd, maxSocket);
            FD_SET(client->sockfd, &readSockets);
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
    if(coCurrentClient->messagePos == coCurrentClient->messageLength) {
        return EOF;
    }
    return coCurrentClient->message[(coCurrentClient->messagePos)++];
}

/*--------------------------------------------------------------------------------------------------
  Read a character from the active client's message buffer.
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
    write(coCurrentClient->sockfd, buffer, length);
    return length;
}

/*--------------------------------------------------------------------------------------------------
  Just send the '\0' to terminate the message.
--------------------------------------------------------------------------------------------------*/
void coCompleteResponse(void)
{
    write(coCurrentClient->sockfd, "", 1);
    coCurrentClient = NULL;
}

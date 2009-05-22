#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>

struct clientStruct;
typedef unsigned int uint;
typedef struct clientStruct *client;

struct clientStruct {
    int sockfd;
    char lineBuf[CO_MAX_MESSAGE_SIZE];
    uint lineSize;
    uint linePos;
    char *sessionId;
    uint hash;
    client next;
};

static client *coClientTable;
static uint coClientTableSize = 32;
static uint coNumClients = 0;
static int coServerSockfd;
static char *coSocketPath;

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
    int         serverLen;
    struct sockaddr_un serverAddress;

    unlink(fileSocketPath);
    coServerSockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    setSocketNonBlocking(coServerSockfd, 1);
    int socket,
    serverAddress.sun_family = AF_UNIX;
    strcpy(serverAddress.sun_path, fileSocketPath);
    serverLen = strlen(serverAddress) + 1;
    if(bind(coServerSockfd, (struct sockaddr*)&serverAddress, serverLen)) {
        perror("failed to bind socket");
        exit(1);
    }
    listen(server_sockfd, 5);
    coSocketPath = calloc(strlen(fileSocketPath) + 1, sizeof(char));
    strcpy(soSocketPath, fileSocketPath);
}

/*--------------------------------------------------------------------------------------------------
  Stop the server.
--------------------------------------------------------------------------------------------------*/
void coStopServer(void)
{
    unlink(soSocketPath);
    free(soSocketPath);
    close(soServerSockfd);
}

/*--------------------------------------------------------------------------------------------------
  This command waits for a completed message from a client, and then returns the sessionId of
  that client.  Any calls to coPrintf after this call will direct the text to that client.
--------------------------------------------------------------------------------------------------*/
char *coStartResponse(void)
{
    for(;;) {
        char ch;

        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);

        read(client_sockfd, &ch, 1);
        ++ch;
        write(client_sockfd, &ch, 1);
        close(client_sockfd);
    }

    fd_set readSockets, writeSockets;
    struct timeval tv;
    int maxSocket = bsSocketGetNumber(bsListeningSocket);
    int numActiveSockets;

    if(bsCommandListeningSocket != bsSocketNull) {
        maxSocket = utMax(maxSocket, bsSocketGetNumber(bsCommandListeningSocket));
    }
    maxSocket = utMax(maxSocket, bsSocketGetNumber(bsInteractiveInputSocket));
    maxSocket = utMax(maxSocket, bsSocketGetNumber(bsInteractiveOutputSocket));
    FD_ZERO(&readSockets);
    FD_ZERO(&writeSockets);
    addWriteMessagesToSet(&writeSockets);
    bsForeachRootTorrent(bsTheRoot, torrent) {
        bsForeachTorrentConnection(torrent, connection) {
            socket = bsConnectionGetSocket(connection);
            maxSocket = utMax(bsSocketGetNumber(socket), maxSocket);
            FD_SET(bsSocketGetNumber(socket), &readSockets);
        } bsEndForeachTorrentConnection;
    } bsEndForeachRootTorrent;
    bsForeachRootPendingConnection(bsTheRoot, connection) {
        socket = bsConnectionGetSocket(connection);
        maxSocket = utMax(bsSocketGetNumber(socket), maxSocket);
        FD_SET(bsSocketGetNumber(socket), &readSockets);
    } bsEndForeachRootPendingConnection;
    FD_SET(bsSocketGetNumber(bsListeningSocket), &readSockets);
    if(bsCommandPort != 0) {
        FD_SET(bsSocketGetNumber(bsCommandListeningSocket), &readSockets);
    }
    if(bsInteractiveInputSocket != bsSocketNull) {
        FD_SET(bsSocketGetNumber(bsInteractiveInputSocket), &readSockets);
    }
    if(bsInteractiveOutputSocket != bsSocketNull && bsOutputBufPos != bsOutputBytesWritten) {
        FD_SET(bsSocketGetNumber(bsInteractiveOutputSocket), &writeSockets);
    }
    /* Wait up to 2 seconds. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    utIfVerbose(3) {
        utDebug("Calling select...\n");
    }
    bsUnlockDatabaseAccess();
    numActiveSockets = select(maxSocket + 1, &readSockets, &writeSockets, NULL, &tv);
    bsLockDatabaseAccess();
    if(numActiveSockets < 0) {
        bsLogLibError("Select error");
        return false;
    }
    sendMessages(writeSockets);
    readMessages(readSockets);
    if(bsCommandPort != 0 && FD_ISSET(bsSocketGetNumber(bsCommandListeningSocket), &readSockets)) {
        bsAcceptCommandConnection();
    }
    if(FD_ISSET(bsSocketGetNumber(bsListeningSocket), &readSockets)) {
        bsAcceptPendingConnection();
    }
    if(bsOutputBufPos != bsOutputBytesWritten &&
            FD_ISSET(bsSocketGetNumber(bsInteractiveOutputSocket), &writeSockets)) {
        writeToInteractiveOutput();
    }
    if(FD_ISSET(bsSocketGetNumber(bsInteractiveInputSocket), &readSockets)) {
        return bsProcessCommands();
    }
    return true;
}

int coPrintf(char *format, ...);
coCompleteResponse(void);
char coGetc(void):

int main(
    int argc,
    char **argv)
{
    int         server_sockfd;
    int         client_sockfd;
    int         server_len;
    int         client_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;

    unlink("server_socket");
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, "server_socket");
    server_len = sizeof(server_address);
    client_len = sizeof(client_address); //get byte length of client address
    bind(server_sockfd, (struct sockaddr*)&server_address, server_len);

    listen(server_sockfd, 5);
    for(;;) {
        char ch;

        // printf("server waiting\n");

        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);

        read(client_sockfd, &ch, 1);
        ++ch;
        write(client_sockfd, &ch, 1);
        close(client_sockfd);
    }
}

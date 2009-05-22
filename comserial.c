/*=================================================================================================
  This is a server-side module which creates a file socket for communication with multiple
  clients, typically Apache scripts responding to an HTTP request.  A simple printf/getc style
  interface is presented to the server which merges all client communication into one input and
  one output stream.  The point is to allow console apps to serve as the back-end engines to
  Apache without the hassle of managing network connections, multiple threads, or 'select' loops.
  Communication between server and client is done one command at a time.

  All messages between client and server are of the form:

      sessionId &text message&

  The sessionId is any string not containing spaces, and is terminated by a single space.  The
  text message is encoded such that ampersands are replaced with % escapes.  However, the user
  does not need to worry about message format, because the server and client communicate through
  higher level functions.

  The client communicates with the server using:

      coSendMessage(char *sessionId, char *format, ...);
      char *coReadMessage(char **sessionIdPtr);

  The server reacts to client messages with:

      char *coStartResponse(void);
      int coPrint(char *format, ...);
      coCompleteResponse(void);
      char coGetc(void):

  The point of these functions is to act more like the functions used in a typical console app:
  printf and getchar.  At the top level of a command interpreter loop, you should call
  coStartResponse, which returns the sessionId of the next message from a client.  Then call your
  command processing code, which calls coGetc until it returns '\n' ('\0' will be the actual
  termination of the messsage).  Instead of calling printf in response, call coPrint.  When
  control returns back to the command processing loop, call coCompleteResponse to cause the
  response message to be sent.  Note that empty responses will result in an empty message, so
  clients should expect a response to every message.

  The server and initialize/closesthings with:

      int coStartServer(char *fileSocketPath);
      void coStopServer(void);

  The client does so with:

      int coStartClient(char *fileSocketPath);
      void coStopClient(void);
=================================================================================================*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>

struct clientStruct;
typedef unsigned int uint;
typedef struct clientStruct *client;

struct clientStruct {
    int socketdf;
    char *lineBuf;
    uint lineSize;
    uint linePos;
    char *sessionId;
    uint hash;
    client next;
};

static client *clientTable;
static uint clientTableSize = 32;

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

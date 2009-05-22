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

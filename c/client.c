#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>


struct client {
    char host[256];
    unsigned short port;
    int socket;     
};

int client_setup(struct client *client)
{
    struct sockaddr_in addr;

    if ((client->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Failed to create socket: %s\n", strerror(errno));
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(client->port);
    addr.sin_addr.s_addr = inet_addr(client->host);

    if (connect(client->socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Failed to connect to the server: %s\n", strerror(errno));
        exit(1);
    }
    return 0;
}

int client_send(struct client *client, const char *method, const char *ip, char *response)
{
    char request[1024];

    sprintf(request, "method:%s\nip:%s\n", method, ip);

    if (send(client->socket, request, strlen(request), 0) < 0) {
        printf("Failed to send request: %s\n", strerror(errno));
        return -1;
    }

    if (recv(client->socket, response, 1023, 0) < 0) {
        if (errno == EAGAIN && errno == EWOULDBLOCK) {
            printf("Connection timed out to %s\n", ip);
        } else {
            printf("Failed to receive response: %s\n", strerror(errno));
        }
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    struct client client;
    char response[1024];

    if (argc < 3) {
        printf("Usage: %s <method> <ip>\n", argv[0]);
        exit(1);
    }

    strncpy(client.host, "127.0.0.1", 256);
    client.port = 5555;
    client_setup(&client);
    client_send(&client, argv[1], argv[2], response);

    printf(response);
}

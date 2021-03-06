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


#include "logging.h"
#include "netpack.h"

#define LOG_LEVEL LOG_INFO
#define LOG_PREFIX 0

#ifndef HOST
    #define HOST "127.0.0.1"
#endif

#ifndef PORT
    #define PORT 5555
#endif


int setup(const char *ip, unsigned short port)
{
    int sock;
    struct sockaddr_in addr;

    log_debug("Connect to %s:%d", ip, port);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("Failed to create socket: %s", strerror(errno));
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_error("Failed to connect to the server: %s", strerror(errno));
        return -1;
    }

    log_debug("Connected to %s:%d", ip, port);
    return sock;
}

int communicate(int sock, char *buffer, size_t size)
{
    log_debug("Send buffer '%s'", buffer);
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        log_error("Failed to send request: %s", strerror(errno));
        return -1;
    }
    log_debug("Buffer has been sent");

    memset(buffer, 0, size); 

    log_debug("Receive buffer");
    if (recv(sock, buffer, size-1, 0) < 0) {
        log_error("Failed to receive response: %s", strerror(errno));
        return -1;
    }
    log_debug("Buffer has been received: '%s'", buffer);

    return 0;
}

int teardown(int sock)
{
    log_debug("Tearing down connection");
    close(sock);
    log_debug("Connection has torn down");
    return 0;
}

int main(int argc, char **argv)
{
    int sock;
    char buffer[1024];
    struct request request;
    struct response response;

    // Solid logging
    log_set(LOG_LEVEL, log_std_prefix);

    if (argc < 3) {
        log_error("Usage: %s <method> <ip>", argv[0]);
        return 1;
    }
    log_debug("argv[0]=%s; argv[1]=%s; argv[2]=%s", argv[0], argv[1], argv[2]);

    memset(buffer, 0, sizeof(buffer));
    memset(&request, 0, sizeof(struct request));
    memset(&response, 0, sizeof(struct response));

    // Create request
    strncpy(request.method, argv[1], sizeof(request.method)-1);
    strncpy(request.ip, argv[2], sizeof(request.ip)-1);
    if (compose_request(buffer, request, sizeof(buffer)) < 0) {
        log_error("Failed to compose reques");
        return 1;
    }

    // Communicate with the server
    if ((sock = setup(HOST, PORT)) < 0)
        return 1;
    if (communicate(sock, buffer, sizeof(buffer)) < 0)
        return 1;

    teardown(sock);

    // Parse response
    if (parse_response(buffer, &response) < 0) {
        log_error("Failed to parse response");
        return 1;
    }

    log_info(response.reason);
    return 0;
}

#include <string.h>
#include <errno.h> //It defines macros for reporting and retrieving error conditions through error codes
#include <unistd.h> //contains various constants
#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate

#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses

#include "logging.h"
#include "connection.h"
#include "runner.h"

void connection_teardown(int connection, const char *ip, unsigned short port);


void connection_handle(int connection, struct sockaddr_in addr)
{
    char ip[40];
    char msg[1024];
    char response[1024];
    unsigned short port;
    struct timeval tv = {5, 0};
    struct request request;


    memset(ip, 0, sizeof(ip));
    memset(msg, 0, sizeof(msg));
    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(msg));

    if (inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)) == NULL) {
        log_error("Failed to parse ip address: %s", strerror(errno));
    }
    port = htons(addr.sin_port);
    log_debug("Connection from %s:%d", ip, port);

    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
    if (recv(connection, msg, sizeof(msg)-1, 0) < 0) {
        if (errno == EAGAIN && errno == EWOULDBLOCK) {
            log_warning("Connection timed out to %s", ip);
        } else {
            log_error("Failed to receive msg: %s", strerror(errno));
        }
        connection_teardown(connection, ip, port);
        return;
    }

    log_debug("Request: \n%s", msg);

    unpack(msg, &request);
    runner_execute(request, &response);
    pack(&text, response);


    if (send(connection, response, strlen(response), 0) < 0) {
        log_error("Failed to send response: %s", strerror(errno));
    }

    connection_teardown(connection, ip, port);
}

void connection_teardown(int connection, const char *ip, unsigned short port)
{
    close(connection);
    log_debug("Connection is closed to %s:%d", ip, port);
}

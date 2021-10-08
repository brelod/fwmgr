#include <string.h>
#include <errno.h> //It defines macros for reporting and retrieving error conditions through error codes
#include <unistd.h> //contains various constants
#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate

#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses

#include "runner.h"
#include "logging.h"
#include "netpack.h"
#include "connection.h"

static void teardown(int sock, const char *ip, unsigned short port);


void con_handler(int sock, struct sockaddr_in addr)
{
    char ip[40];
    unsigned short port;

    char buffer[1024];                                                  // TODO: netpack --> BUFFER_SIZE
    struct timeval tv = {5, 0};                                         // TODO: use preproc TIMEOUT
    struct request request;
    struct response response;

    memset(ip, 0, sizeof(ip));
    memset(buffer, 0, sizeof(buffer));
    memset(&request, 0, sizeof(struct request));
    memset(&response, 0, sizeof(struct response));

    if (inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)) == NULL) {
        log_error("Failed to parse ip address: %s", strerror(errno));
    }
    port = htons(addr.sin_port);
    log_debug("Connection from %s:%d", ip, port);

    // Receive request
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
    if (recv(sock, buffer, sizeof(buffer)-1, 0) < 0) {
        if (errno == EAGAIN && errno == EWOULDBLOCK) {
            log_warning("Connection timed out to %s", ip);
        } else {
            log_error("Failed to receive msg: %s", strerror(errno));
        }
        teardown(sock, ip, port);
        return;
    }

    // Process request
    log_debug("Request: \n%s", buffer);
    parse_request(buffer, &request);

    // Create response
    memset(buffer, 0, sizeof(buffer));
    response.code = 0;
    strcpy(response.reason, "<teplate-reason>");
    compose_response(buffer, response, sizeof(buffer));

    // Send response
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        log_error("Failed to send response: %s", strerror(errno));
    }

    teardown(sock, ip, port);
}

static void teardown(int sock, const char *ip, unsigned short port)
{
    close(sock);
    log_debug("Connection is closed to %s:%d", ip, port);
}

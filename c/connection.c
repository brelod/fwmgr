#include <string.h>
#include <errno.h> //It defines macros for reporting and retrieving error conditions through error codes
#include <unistd.h> //contains various constants
#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate

#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses
#include <stdlib.h>
#include <stdio.h>

#include "runner.h"
#include "logging.h"
#include "netpack.h"
#include "connection.h"

static void teardown(struct connection *con);


void con_handler(void *arg)
{
    struct connection *con = (struct connection*) arg;
    struct timeval timeout = {5, 0};

    struct request request;
    struct response response;
    char buffer[sizeof(response.reason)];

    memset(buffer, 0, sizeof(buffer));
    memset(&request, 0, sizeof(struct request));
    memset(&response, 0, sizeof(struct response));

    log_debug("Connection from %s:%d", con->ip, con->port);

    // Receive request
    setsockopt(con->socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*) &timeout, sizeof(timeout));
    if (recv(con->socket, buffer, sizeof(buffer)-1, 0) < 0) {
        log_error("Failed to receive request: %s", strerror(errno));
        teardown(con);
        return;
    }

    // Process request
    log_debug("Request: '%s'", buffer);
    parse_request(buffer, &request);

    // Execute subprocess
    if (runner_process(request, &response) < 0) {
        response.code = 1;
        snprintf(response.reason, sizeof(response.reason), "Internal error (See server logs)");
    }

    // Create response
    memset(buffer, 0, sizeof(buffer));
    compose_response(buffer, response, sizeof(buffer));
    log_debug("Response: '%s'", buffer);

    // Send response
    if (send(con->socket, buffer, strlen(buffer), 0) < 0) {
        log_error("Failed to send response: %s", strerror(errno));
    }

    teardown(con);
}

static void teardown(struct connection *con)
{
    log_debug("Close connection to %s:%d", con->ip, con->port);
    close(con->socket);
    free(con);
}

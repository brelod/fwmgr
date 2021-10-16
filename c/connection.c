#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

#include "runner.h"
#include "logging.h"
#include "netpack.h"
#include "connection.h"

static void teardown(session_t *session);


void con_handler(void *arg)
{
    session_t *session = (session_t*) arg;
    struct timeval timeout = {5, 0};

    struct request request;
    struct response response;
    char buffer[sizeof(response.reason)];

    memset(buffer, 0, sizeof(buffer));
    memset(&request, 0, sizeof(struct request));
    memset(&response, 0, sizeof(struct response));

    log_debug("Connection from %s:%d", session->ip, session->port);

    // Receive request
    setsockopt(session->socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*) &timeout, sizeof(timeout));
    if (recv(session->socket, buffer, sizeof(buffer)-1, 0) < 0) {
        log_error("Failed to receive request: %s", strerror(errno));
        teardown(session);
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
    if (send(session->socket, buffer, strlen(buffer), 0) < 0) {
        log_error("Failed to send response: %s", strerror(errno));
    }

    teardown(session);
}

static void teardown(session_t *session)
{
    log_debug("Close connection to %s:%d", session->ip, session->port);
    close(session->socket);
}

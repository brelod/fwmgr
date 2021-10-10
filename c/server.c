#include <string.h>
#include <errno.h> //It defines macros for reporting and retrieving error conditions through error codes
#include <unistd.h> //contains various constants
#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate

#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses

#include <signal.h>
#include <stdlib.h>

#include "logging.h"
#include "server.h"
#include "connection.h"
#include "threadpool.h"


#ifndef THREADS
    #define THREADS 4
#endif

#ifndef QUEUE_SIZE
    #define QUEUE_SIZE 256
#endif

#ifndef HOST
    #define HOST "127.0.0.1"
#endif

#ifndef PORT
    #define PORT 5555
#endif


struct server {
    tp_t *tp;
    int socket;
    struct sockaddr_in addr;
};


struct server* setup(const char *ip, unsigned short port)
{
    int sock;
    struct sockaddr_in addr;
    struct server *server = NULL;

    log_info("Starting server on %s:%d", ip, port);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("Failed to create socket: %s", strerror(errno));
        return NULL;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        log_error("Failed to set socket option (SO_REUSEADDR): %s", strerror(errno));
        return NULL;
    }

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_error("Failed to bind socket: %s", strerror(errno));
        return NULL;
    }

    if (listen(sock, 5) < 0) {
        log_error("Failed to listen on socket: %s", strerror(errno));
        return NULL;
    }

    server = (struct server*) calloc (1, sizeof(struct server));
    if (server < 0) {
        return NULL;
    }
    server->socket = sock;
    server->addr = addr;
    server->tp = tp_create(THREADS, QUEUE_SIZE);

    if (server->tp == NULL) {
        free(server);
        return NULL;
    }

    tp_start(server->tp);

    log_info("Server has started");
    return server;
}

int operate(struct server *server)
{
    int con;
    tp_job_t *job = NULL;
    struct sockaddr_in addr;
    struct connection *connection = NULL;
    socklen_t size = sizeof(addr);

    log_info("Start listening");
    while (1) {
        connection = (struct connection*) calloc (1, sizeof(struct connection));
        if (connection < 0) {
            log_error("Failed to calloc memory for connection");
            return -1;
        }
        job = (tp_job_t*) calloc (1, sizeof(tp_job_t));
        if (job < 0) {
            log_error("Failed to calloc memory for job");
            return -1;
        }

        con = accept(server->socket, (struct sockaddr*)&addr, &size);
        if (con < 0) {
            log_error("Failed to accept connection: %s", strerror(errno));
            return -1;
        }

        connection->socket = con;
        connection->addr = addr;
        job->function = con_handler;
        job->arg = (void*)connection;
        tp_job_run(server->tp, job);
    }
    log_info("Stop listening");
    return 0;
}

int teardown(struct server *server)
{
    log_info("Stopping server");
    close(server->socket);
    tp_stop(server->tp);
    tp_destroy(server->tp);
    free(server);
    log_info("Server is stopped");
    return 0;
}

void interrupt_handler(int sig)
{
    signal(sig, SIG_IGN);
    log_info("Stopping server because of KeyboardInterrupt");
    signal(SIGINT, SIG_DFL);
    raise(sig);
}


int main(int argc, char **argv)
{
    struct server *server = NULL;

    if (argc > 1 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0))
        log_set(LOG_DEBUG, 1);
    else
        log_set(LOG_INFO, 1);


    signal(SIGINT, interrupt_handler);


    if ((server = setup(HOST, PORT)) == NULL)
        return 1;

    operate(server);
    teardown(server);

    return 0;
}

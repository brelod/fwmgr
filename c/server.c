#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <signal.h>
#include <stdlib.h>

#include "logging.h"
#include "server.h"
#include "connection.h"
#include "threadpool.h"


#ifndef THREADS
#       define THREADS 4
#endif

#ifndef QUEUE_SIZE
#       define QUEUE_SIZE 256
#endif

#ifndef HOST
#       define HOST "127.0.0.1"
#endif

#ifndef PORT
#       define PORT 5555
#endif

#define QUEUE_WAIT 10000


struct server {
        tp_t *tp;
        int socket;
        struct sockaddr_in addr;
        session_t *sessions;
};


static struct server server;

int setup(const char *ip, unsigned short port)
{
        int sock;
        struct sockaddr_in addr;
        tp_job_t *job;

        log_info("Starting server on %s:%d", ip, port);

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                log_error("Failed to create socket: %s", strerror(errno));
                return -1;
        }

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
                log_error("Failed to set socket option (SO_REUSEADDR): %s", strerror(errno));
                return -1;
        }

        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                log_error("Failed to bind socket: %s", strerror(errno));
                return -1;
        }

        if (listen(sock, 5) < 0) {
                log_error("Failed to listen on socket: %s", strerror(errno));
                return -1;
        }

        server.socket = sock;
        server.addr = addr;
        server.tp = tp_create(THREADS, QUEUE_SIZE);

        if (server.tp == NULL) {
                log_error("Failed to create threadpool");
                return -1;
        }

        // Init queue with connection objects
        server.sessions = (session_t*) calloc (QUEUE_SIZE, sizeof(*server.sessions));
        if (server.sessions == NULL) {
                log_error("Failed to calloc() sessions");
                return -1;
        }

        for (int i=0; i<QUEUE_SIZE; ++i) {
                job = (tp_job_t*) server.tp->jobs.finished->nodes[i];
                job->function = con_handler;
                job->arg = (void*) &server.sessions[i];
        }

        tp_start(server.tp);

        log_info("Server has started");
        return 0;
}

int operate()
{
        tp_job_t *job = NULL;
        struct sockaddr_in addr;
        session_t *session = NULL;
        socklen_t size = sizeof(addr);

        log_info("Start listening");
        while (1) {
                // Find a session to overwrite
                while ((job = tp_get(server.tp)) == NULL) {
                        log_warning("No free job is available");
                        usleep(QUEUE_WAIT);
                }
                session = (session_t*) job->arg;
                
                // Update the session
                session->socket = accept(server.socket, (struct sockaddr*)&addr, &size);
                if (session->socket < 0) {
                        log_error("Failed to accept connection: %s", strerror(errno));
                        return -1;
                }

                if (inet_ntop(AF_INET, &addr.sin_addr, session->ip, sizeof(session->ip)) == NULL) {
                        log_warning("Failed to parse ip address: %s", strerror(errno));
                        return -1;
                }
                session->port = htons(addr.sin_port);

                // Start the job with the session
                while(tp_put(server.tp, job) < 0) {
                        log_warning("Job queue overflow");
                        usleep(QUEUE_WAIT);
                }
        }
        log_info("Stop listening");
        return 0;
}

int teardown()
{
    log_info("Stopping server");
    close(server.socket);
    free(server.sessions);
    tp_stop(server.tp);
    tp_destroy(server.tp);
    log_info("Server is stopped");
    return 0;
}

void interrupt_handler(int sig)
{
    signal(sig, SIG_IGN);
    teardown();
    signal(SIGINT, SIG_DFL);
    raise(sig);
}


int main(int argc, char **argv)
{
    if (argc > 1 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0))
        log_set(LOG_DEBUG, log_std_prefix);
    else
        log_set(LOG_INFO, log_std_prefix);


    signal(SIGINT, interrupt_handler);

        log_debug("debug");
        log_info("debug");
        log_warning("debug");
        log_error("debug");
        log_trace();

    if (setup(HOST, PORT) < 0)
        return 1;

    operate();
    teardown();

    return 0;
}

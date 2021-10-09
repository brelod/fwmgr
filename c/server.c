#include <string.h>
#include <errno.h> //It defines macros for reporting and retrieving error conditions through error codes
#include <unistd.h> //contains various constants
#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate

#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses

#include <signal.h>

#include "logging.h"
#include "server.h"
#include "connection.h"


#define HOST "127.0.0.1"
#define PORT 5555


int setup(const char *ip, unsigned short port)
{
    int sock;
    struct sockaddr_in addr;

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

    log_info("Server has started");
    return sock;
}

int operate(int sock)
{
    int con;
    struct sockaddr_in addr;
    socklen_t size = sizeof(addr);

    log_info("Start listening");
    while (1) {
        con = accept(sock, (struct sockaddr*)&addr, &size);
        if (con < 0) {
            log_error("Failed to accept connection: %s", strerror(errno));
            return -1;
        }

        con_handler(con, addr);
    }
    log_info("Stop listening");
    return 0;
}

int teardown(int sock)
{
    log_info("Stopping server");
    close(sock);
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
    int sock;

    if (argc > 1 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0))
        log_set(LOG_DEBUG, 1);
    else
        log_set(LOG_INFO, 1);


    signal(SIGINT, interrupt_handler);


    if ((sock = setup(HOST, PORT)) < 0)
        return 1;

    operate(sock);
    teardown(sock);

    return 0;
}

#include <string.h>
#include <errno.h> //It defines macros for reporting and retrieving error conditions through error codes
#include <unistd.h> //contains various constants
#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate

#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses

#include <signal.h>

#include "logging.c"


static int server_socket;



void connection_handle(int connection, struct sockaddr_in addr)
{
    char ip[40];
    unsigned short port;

    if (inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)) == NULL) {
        log_error("Failed to parse ip address: %s", strerror(errno));
    }
    port = htons(addr.sin_port);
    log_debug("Connection from %s:%d", ip, port);
}

int server_setup(const char *ip, unsigned short port)
{
    struct sockaddr_in addr;

    log_info("Starting server");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("Failed to create socket: %s", strerror(errno));
        return -1;
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        log_error("Failed to set socket option (SO_REUSEADDR): %s", strerror(errno));
        return -1;
    }
    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_error("Failed to bind socket: %s", strerror(errno));
        return -1;
    }
    if (listen(server_socket, 1) < 0) {
        log_error("Failed to listen on socket: %s", strerror(errno));
        return -1;
    }

    log_info("Server has started");
}

int server_listen()
{
    int connection;
    struct sockaddr_in addr;
    socklen_t size = sizeof(addr);

    log_info("Start listening");
    while (1) {
        connection = accept(server_socket, (struct sockaddr*)&addr, &size);
        if (connection < 0) {
            log_error("Failed to accept connection: %s", strerror(errno));
            return -1;
        }

        connection_handle(connection, addr);
    }
    log_info("Stop listening");
    return 0;
}

int server_teardown()
{
    log_info("Stopping server");
    close(server_socket);
    log_info("Server is stopped");
    return 0;
}

void interrupt_handler(int sig)
{
    signal(sig, SIG_IGN);
    log_info("Stopping server because of KeyboardInterrupt");
    server_teardown();
}

int server_run(const char *addr, unsigned short port)
{
    signal(SIGINT, interrupt_handler);

    if (server_setup(addr, port) < 0) {
        return -1;
    }
    server_listen();
    server_teardown();
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 1 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0))
        set_loglevel(DEBUG);
    else
        set_loglevel(INFO);


    server_run("localhost", 5555);
    return 0;
}

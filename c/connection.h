#ifndef CONNECTION_H
#define CONNECTION_H


#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate

#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses


struct connection {
    int socket;
    struct sockaddr_in addr;
};


void con_handler(void *arg);
#endif


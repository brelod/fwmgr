#ifndef CONNECTION_H
#define CONNECTION_H


#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


struct connection {
    int socket;
    char ip[40];
    unsigned short port;
};


void con_handler(void *arg);
#endif


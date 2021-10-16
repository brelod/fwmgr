#pragma once

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


typedef struct session {
    int socket;
    char ip[40];
    unsigned short port;
} session_t;


void con_handler(void *arg);

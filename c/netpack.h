#ifndef NETPACK_H
#define NETPACK_H

typedef unsigned char uint8_t;

struct request {
    char method[256];
    char ip[256];
};

struct response {
    uint8_t code;
    char reason[512];
};

int parse_request(const char *text, struct request *request);
int parse_response(const char *text, struct response *response);
int compose_request(char *text, struct request request, size_t size);
int compose_response(char *text, struct response response, size_t size);

#endif

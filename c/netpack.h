#pragma once

typedef unsigned char uint8_t;

#define REQUEST_METHOD_SIZE 256
#define REQUEST_IP_SIZE 40
#define RESPONSE_REASON_SIZE 1024

struct request {
    char method[256];
    char ip[40];
};

struct response {
    int code;
    char reason[1024];
};

int parse_request(const char *text, struct request *request);
int parse_response(const char *text, struct response *response);
int compose_request(char *text, struct request request, size_t size);
int compose_response(char *text, struct response response, size_t size);

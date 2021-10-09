#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "netpack.h"
#include "logging.h"


#define DELIM_PAIR ";"
#define DELIM_KEYVAL "="
#define chr2num(chr) (uint8_t)(chr) & 0xf
#define num2chr(num) (char)(num) | 0x30


int parse_request(const char *text, struct request *request)
{
    char *buffer, *pair, *key, *val;
    char *save_pair, *save_keyval;

    buffer = strdup(text);
    memset(request, 0, sizeof(struct request));

    pair = strtok_r(buffer, DELIM_PAIR, &save_pair);
    while (pair != NULL) {
        key = strtok_r(pair, DELIM_KEYVAL, &save_keyval);
        val = strtok_r(NULL, DELIM_KEYVAL, &save_keyval);

        if (key == NULL || val == NULL)                                     // TODO: does this work ???
            return -1;

        if (strcmp(key, "method") == 0) {
            strncpy(request->method, val, sizeof(request->method));
        } else if (strcmp(key, "ip") == 0) {
            strncpy(request->ip, val, sizeof(request->ip));
        }

        pair = strtok_r(NULL, DELIM_PAIR, &save_pair);
    }

    free(buffer);
    return 0;
}

int parse_response(const char *text, struct response *response)
{
    char *buffer, *pair, *key, *val;
    char *save_pair, *save_keyval;

    buffer = strdup(text);
    memset(response, 0, sizeof(struct response));

    pair = strtok_r(buffer, DELIM_PAIR, &save_pair);
    while (pair != NULL) {
        key = strtok_r(pair, DELIM_KEYVAL, &save_keyval);
        val = strtok_r(NULL, DELIM_KEYVAL, &save_keyval);

        if (key == NULL || val == NULL)                                     // TODO: check this
            return -1;

        if (strcmp(key, "code") == 0) {
            response->code = chr2num(val[0]);
        } else if (strcmp(key, "reason") == 0) {
            strncpy(response->reason, val, sizeof(response->reason));
        }

        pair = strtok_r(NULL, DELIM_PAIR, &save_pair);
    }

    free(buffer);
    return 0;
}

int compose_request(char *text, struct request request, size_t size)
{
    return snprintf(text, size, 
                    "method" DELIM_KEYVAL "%s" DELIM_PAIR
                    "ip" DELIM_KEYVAL "%s",
                    request.method, request.ip);
}

int compose_response(char *text, struct response response, size_t size)
{
    return snprintf(text, size, 
                    "code" DELIM_KEYVAL "%d" DELIM_PAIR
                    "reason" DELIM_KEYVAL "%s",
                    response.code, response.reason);
}

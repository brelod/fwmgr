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

    log_debug("Parsing request: '%s'", text);

    buffer = strdup(text);

    pair = strtok_r(buffer, DELIM_PAIR, &save_pair);
    while (pair != NULL) {
        key = strtok_r(pair, DELIM_KEYVAL, &save_keyval);
        val = strtok_r(NULL, DELIM_KEYVAL, &save_keyval);

        if (key == NULL || val == NULL) {
            log_warning("Failed to parse request: key='%s', val='%s'", key, val);
            return -1;
        }

        if (strcmp(key, "method") == 0) {
            snprintf(request->method, sizeof(request->method), "%s", val);
        } else if (strcmp(key, "ip") == 0) {
            snprintf(request->ip, sizeof(request->ip), "%s", val);
        }

        pair = strtok_r(NULL, DELIM_PAIR, &save_pair);
    }

    log_debug("Parsed request: method='%s', ip='%s'", request->method, request->ip);
    free(buffer);
    return 0;
}

int parse_response(const char *text, struct response *response)
{
    char *buffer, *pair, *key, *val;
    char *save_pair, *save_keyval;

    log_debug("Parsing response: '%s'", text);

    buffer = strdup(text);

    pair = strtok_r(buffer, DELIM_PAIR, &save_pair);
    while (pair != NULL) {
        key = strtok_r(pair, DELIM_KEYVAL, &save_keyval);
        val = strtok_r(NULL, DELIM_KEYVAL, &save_keyval);

        if (key == NULL || val == NULL) {
            log_warning("Failed to parse response: key='%s', val='%s'", key, val);
            return -1;
        }

        if (strcmp(key, "code") == 0) {
            response->code = atoi(val);
        } else if (strcmp(key, "reason") == 0) {
            snprintf(response->reason, sizeof(response->reason), "%s", val);
        }

        pair = strtok_r(NULL, DELIM_PAIR, &save_pair);
    }

    log_debug("Parsed response code='%d', reason='%s'", response->code, response->reason);
    free(buffer);
    return 0;
}

int compose_request(char *text, struct request request, size_t size)
{
    int bytes;
    log_debug("Composing request method='%s', ip='%s'", request.method, request.ip);

    bytes = snprintf(text, size, 
                    "method" DELIM_KEYVAL "%s" DELIM_PAIR
                    "ip" DELIM_KEYVAL "%s",
                    request.method, request.ip);

    log_debug("Composed request: '%s'", text);
    return bytes;
}

int compose_response(char *text, struct response response, size_t size)
{
    int bytes;
    log_debug("Composing response code='%d', reason='%s'", response.code, response.reason);

    bytes = snprintf(text, size, 
                    "code" DELIM_KEYVAL "%d" DELIM_PAIR
                    "reason" DELIM_KEYVAL "%s",
                    response.code, response.reason);

    log_debug("Composed request: '%s'", text);
    return bytes;
}

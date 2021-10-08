#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "netpack.h"




#define DELIM_LINE "\r\n"
#define DELIM_DICT ":"
#define chr2num(chr) (uint8_t)(chr) & 0xf
#define num2chr(num) (char)(num) | 0x30



int parse_request(const char *text, struct request *request)
{
    char *buffer, *line, *key, *val;
    char *save_line, *save_dict;

    buffer = strdup(text);
    memset(request, 0, sizeof(struct request));

    line = strtok_r(buffer, DELIM_LINE, &save_line);
    while (line != NULL) {
        key = strtok_r(line, DELIM_DICT, &save_dict);
        val = strtok_r(NULL, DELIM_DICT, &save_dict);

        if (key == NULL || val == NULL)                                     // TODO: does this work ???
            return -1;

        if (strcmp(key, "method") == 0) {
            strncpy(request->method, val, sizeof(request->method));
        } else if (strcmp(key, "ip") == 0) {
            strncpy(request->ip, val, sizeof(request->ip));
        }

        line = strtok_r(NULL, DELIM_LINE, &save_line);
    }

    free(buffer);
    return 0;
}

int parse_response(const char *text, struct response *response)
{
    char *buffer, *line, *key, *val;
    char *save_line, *save_dict;

    buffer = strdup(text);
    memset(response, 0, sizeof(struct response));

    line = strtok_r(buffer, DELIM_LINE, &save_line);
    while (line != NULL) {
        key = strtok_r(line, DELIM_DICT, &save_dict);
        val = strtok_r(NULL, DELIM_DICT, &save_dict);

        if (key == NULL || val == NULL)                                     // TODO: check this
            return -1;

        if (strcmp(key, "code") == 0) {
            response->code = chr2num(val[0]);
        } else if (strcmp(key, "reason") == 0) {
            strncpy(response->reason, val, sizeof(response->reason));
        }

        line = strtok_r(NULL, DELIM_LINE, &save_line);
    }

    free(buffer);
    return 0;
}

int compose_request(char *text, struct request request, size_t size)
{
    return snprintf(text, size, 
                    "method:%s"DELIM_LINE\
                    "ip:%s"DELIM_LINE DELIM_LINE,
                    request.method, request.ip
    );
}

int compose_response(char *text, struct response response, size_t size)
{
    return snprintf(text, size, 
                    "code:%d"DELIM_LINE\
                    "reason:%s"DELIM_LINE DELIM_LINE, 
                    response.code, response.reason
    );
}

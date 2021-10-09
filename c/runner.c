#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "netpack.h"


void runner_process(struct request request, struct response *response)
{
    int code;
    char cmd[1024];
    char success[1024];

    if (strcmp(request.method, "append") == 0) {
        snprintf(cmd, 1023, "iptables -A FORWARD -s '%s' -j ACCEPT", request.ip);
        snprintf(success, 1023, "%s was successfully added", request.ip);
    } else if (strcmp(request.method, "remove") == 0) {
        snprintf(cmd, 1023, "iptables -D FORWARD -s '%s' -j ACCEPT", request.ip);
        snprintf(success, 1023, "%s was successfully removed", request.ip);
    } else {
        response->code = 1;
        snprintf(response->reason, 1023, "Invalid method: '%s'", request.method);
        return;
    }

    code = system(cmd);
    if ((code = system(cmd)) == 0)
        snprintf(response->reason, 1024, success);
    else
        snprintf(response->reason, 1023, "Unexpected error");

    response->code = code;
}

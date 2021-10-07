#include <string.h>


int runner_process(const char *msg, char *response)
{
    char *line, *field;

    line = strtok(msg, "\n");
    while (line != NULL) {
        key = strtok(line, ":");
        value = strtok(line, ":");
    }

    strncpy(response, "hello dolly", 20);
    return 0;
}

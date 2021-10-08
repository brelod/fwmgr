#include <string.h>
#include <stdio.h>



int runner_process(const char *msg, char *response)
{
    char *copy, *line, *key, *value;

    copy = strdup(msg);

    while ((line = strtok(copy, "\n")) != NULL) {
        key = strtok(line, ":");
        value = strtok(line, ":");
        printf("----%s: %s----\n", key, value);
    }

    strncpy(response, "<response template>", 100);
    return 0;
}

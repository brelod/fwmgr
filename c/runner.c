#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>

#include "logging.h"
#include "netpack.h"


#define NUM255 "([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"
#define IPV4_PATTERN "^" NUM255 "." NUM255 "." NUM255 "." NUM255 "$"

void runner_process(struct request request, struct response *response)
{
    int fd[2];
    int status;
    pid_t pid;
    regex_t regex;
    char cmd[1024];
    char buffer[1024];
    char success[1024];
    char append[] = "-A";
    char remove[] = "-D";
    char *argv[] = {"iptables", "<-A/-D>", "FORWARD", "-s", "<ip>", "-j", "ACCEPT", 0};


    memset(cmd, 0, sizeof(cmd));
    memset(buffer, 0, sizeof(buffer));
    memset(success, 0, sizeof(success));

    // Check method
    if (strcmp(request.method, "append") == 0) {
        argv[1] = append;
        snprintf(success, 1023, "%s was successfully added", request.ip);
    } else if (strcmp(request.method, "remove") == 0) {
        argv[1] = remove;
        snprintf(success, 1023, "%s was successfully removed", request.ip);
    } else {
        response->code = 1;
        snprintf(response->reason, 1023, "Invalid method: '%s'", request.method);
        return;
    }

    // Check IP
    if (regcomp(&regex, IPV4_PATTERN, REG_EXTENDED)) {
        log_error("Failed to compile regex");
        exit(1);
    }
    if (regexec(&regex, request.ip, 0, NULL, 0) != 0) {
        log_error("Invalid ip address '%s'", request.ip);
        response->code = 1;
        snprintf(response->reason, 1023, "Invalid ip: '%s'", request.ip);
        return;
    }

    argv[4] = request.ip;

    // Concatenate command
    int counter = 0;
    for (int i=0; argv[i] != NULL; ++i) {
        if ((counter += strlen(argv[i])) > 1024)
            break;
        strcat(cmd, argv[i]);
        strcat(cmd, " ");
    }
    cmd[counter-1] = 0;

    if (pipe(fd) < 0) {
        log_error("Failed to open pipe to fd1");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        log_error("Failed to fork");
        exit(1);
    }
    else if (pid == 0) {
        // Child process
        dup2(fd[1], STDERR_FILENO);
        close(fd[0]);
        log_info("Execute cmd: '%s'", cmd);
        execvp(argv[0], argv);
    } else {
        // Parent process
        close(fd[1]);

        if (read(fd[0], buffer, sizeof(buffer)-1) < 0) {
           perror("Failed to read from stderr of subprocess");
           exit(1);
        }

        // Strip new lines
        for (int i=strlen(buffer)-1; i>=0 && buffer[i] == '\n'; --i)
            buffer[i] = '\0';

        if (waitpid(pid, &status, WUNTRACED) < 0) {
            perror("Waitpid");
            exit(1);
        }

        response->code = status;
        if (status == 0) {
            strncpy(response->reason, success, 1024);
        } else {
            strncpy(response->reason, buffer, 1024);
        }
    }
    
}

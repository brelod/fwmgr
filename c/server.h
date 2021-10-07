#ifndef SERVER_H
#define SERVER_H

void interrupt_handler(int sig);

int server_setup(const char *ip, unsigned short port);
int server_listen();
int server_teardown();
int server_run(const char *addr, unsigned short port);

#endif

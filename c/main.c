#include <string.h>

#include "logging.h"
#include "server.h"


int main(int argc, char **argv)
{
    if (argc > 1 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0))
        set_loglevel(DEBUG);
    else
        set_loglevel(INFO);

    server_run("localhost", 5555);
    return 0;
}

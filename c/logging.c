#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "logging.h"



static char *log_level_names[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
};

struct config {
    enum log_level level;
    int prefix;
};

static struct config config = {
    INFO,
    1,
};

void _log(enum log_level level, const char *fmt, va_list args)
{
    char prefix[1024];
    char timestamp[512];
    time_t t = time(NULL); 

    if (level >= config.level) {
        if (config.prefix) {
            strftime(timestamp, 128, "%Y-%m-%d %H:%M:%S", localtime(&t));
            sprintf(prefix, "%s | (threadname) | %7s | %s\n", timestamp, log_level_names[level], fmt);

        } else {
            snprintf(prefix, sizeof(prefix)-2, "%s\n", fmt);
        }

        vprintf(prefix, args);
    }
}

void log_set(enum log_level level, int prefix)
{
    config.level = level;
    config.prefix = prefix;
}

void log_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(DEBUG, fmt, args);
    va_end(args);
}

void log_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(INFO, fmt, args);
    va_end(args);
}

void log_warning(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(WARNING, fmt, args);
    va_end(args);
}

void log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(ERROR, fmt, args);
    va_end(args);
}

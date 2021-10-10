#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>

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

static struct config config = {LOG_INFO, 1};

void _log(enum log_level level, const char *fmt, va_list args)
{
    char format[1024];
    char timestamp[20];
    time_t t = time(NULL); 

    memset(format, 0, sizeof(format));
    memset(timestamp, 0, sizeof(timestamp));

    if (level >= config.level) {
        if (config.prefix) {
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&t));
            snprintf(format, sizeof(format), "%s | Thread-%lu | %7s | %s\n", timestamp, pthread_self(), log_level_names[level], fmt);

        } else {
            snprintf(format, sizeof(format), "%s\n", fmt);
        }

        vprintf(format, args);
    }
}

void log_set(enum log_level level, int prefix)
{
    config.level = level;
    config.prefix = prefix;
}

void _log_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(LOG_DEBUG, fmt, args);
    va_end(args);
}

void _log_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(LOG_INFO, fmt, args);
    va_end(args);
}

void _log_warning(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(LOG_WARNING, fmt, args);
    va_end(args);
}

void _log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(LOG_ERROR, fmt, args);
    va_end(args);
}

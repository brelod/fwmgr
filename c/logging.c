#include <time.h>
#include <stdio.h>
#include <stdarg.h>



enum loglevel {DEBUG, INFO, WARNING, ERROR};
static char *loglevel_names[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
};

struct config {
    enum loglevel level;
    char format[512];
    int fd;
};

static struct config config = {
    INFO,
    1,
};

void set_loglevel(enum loglevel level)
{
    config.level = level;
}

void _log(enum loglevel level, const char *fmt, va_list args)
{
    time_t t = time(NULL); 
    char timestamp[512];
    char prefix[512];
    char end[512];

    if (level >= config.level) {
        strftime(timestamp, 128, "%Y-%m-%d %H:%M:%S", localtime(&t));
        sprintf(prefix, "%s | (threadname) | %7s | %s\n", timestamp, loglevel_names[level], fmt);
        vprintf(prefix, args);
    }
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

void log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log(ERROR, fmt, args);
    va_end(args);
}

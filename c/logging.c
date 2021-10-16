#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <execinfo.h>

#define LOG_ENABLE

#include "logging.h"

#define LOG_PREFIX_SIZE 1024
#define LOG_BACKTRACE_SIZE 1024


typedef struct {
        enum log_level level;
        int (*prefix)(enum log_level level, const char *file, int line, char *fmt, size_t size);
} config_t;

static config_t config = {LOG_INFO, log_std_prefix};

const char *log_level_names[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "TRACE",
};

int log_no_prefix(enum log_level level, const char *file, int line, char *fmt, size_t size)
{
        return 0;
}

int log_std_prefix(enum log_level level, const char *file, int line, char *fmt, size_t size)
{
        char timestamp[20];
        char location[512];
        time_t t = time(NULL); 

        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&t));
        snprintf(location, 512, "%s:%d", file, line);
        return snprintf(fmt, size, "%s | %7s | %-20s | ", 
                        timestamp, log_level_names[level], location);
}

void log_set(enum log_level level, int (*prefix)(
        enum log_level level, const char *file, int line, char *fmt, size_t size))
{
        config.level = level;
        config.prefix = prefix;
}

void log_write(enum log_level level, const char *file, int line, const char *fmt, ...)
{
        va_list args;
        char prefix[LOG_PREFIX_SIZE];
        char format[LOG_PREFIX_SIZE+1];
        memset(prefix, 0, sizeof(prefix));
        memset(format, 0, sizeof(format));

        if (level >= config.level) {
                config.prefix(level, file, line, prefix, LOG_PREFIX_SIZE);
                snprintf(format, LOG_PREFIX_SIZE+1, "%s%s\n", prefix, fmt);

                va_start(args, fmt);
                vprintf(format, args);
                va_end(args);
        }
}

void _log_trace(const char *file, int line)
{
        int sframes;
        void *bt[LOG_BACKTRACE_SIZE];
        char **lines;

        sframes = backtrace(bt, LOG_BACKTRACE_SIZE);
        lines = backtrace_symbols(bt, sframes);

        for (int i=0; i<sframes; ++i) {
                log_write(LOG_TRACE, file, line, "%s", lines[i]);
        }
}

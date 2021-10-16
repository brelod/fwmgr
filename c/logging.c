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

static config_t config = {LOG_INFO, std_prefix};


int no_prefix(enum log_level level, const char *file, int line, char *fmt, size_t size)
{
        return 0;
}

int std_prefix(enum log_level level, const char *file, int line, char *fmt, size_t size)
{
        char timestamp[20];
        time_t t = time(NULL); 

        memset(timestamp, 0, sizeof(timestamp));

        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&t));
        return snprintf(fmt, size, "%s | %7s | %s:%d | ", 
                        timestamp, log_level_names[level], file, line);
}

void log_set(enum log_level level, int (*prefix)(
        enum log_level level, const char *file, int line, char *fmt, size_t size))
{
        config.level = level;
        config.prefix = prefix;
}

void _log(enum log_level level, const char *file, int line, const char *fmt, ...)
{
        va_list args;
        char prefix[LOG_PREFIX_SIZE];
        char format[LOG_PREFIX_SIZE+3];
        memset(prefix, 0, sizeof(prefix));
        memset(format, 0, sizeof(format));

        if (level >= config.level) {
                config.prefix(level, file, line, prefix, LOG_PREFIX_SIZE - 3);
                snprintf(format, LOG_PREFIX_SIZE, "%s%s\n", prefix, fmt);

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
                _log(LOG_TRACE, file, line, "%s", lines[i]);
        }
}

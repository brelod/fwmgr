#pragma once

#include <stddef.h>

enum log_level {
        LOG_DEBUG, 
        LOG_INFO, 
        LOG_WARNING, 
        LOG_ERROR, 
        LOG_TRACE
};

static char *log_level_names[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "TRACE",
};

int no_prefix(enum log_level level, const char *file, int line, char *fmt, size_t size);
int std_prefix(enum log_level level, const char *file, int line, char *fmt, size_t size);

void log_set(enum log_level level, int (*prefix)(enum log_level level, const char *file, int line, char *fmt, size_t size));

#if defined LOG_ENABLE
        #define log_debug(...)   _log(LOG_DEBUG  , __FILE__, __LINE__, __VA_ARGS__)
        #define log_info(...)    _log(LOG_INFO   , __FILE__, __LINE__, __VA_ARGS__)
        #define log_warning(...) _log(LOG_WARNING, __FILE__, __LINE__, __VA_ARGS__)
        #define log_error(...)   _log(LOG_ERROR  , __FILE__, __LINE__, __VA_ARGS__)
        #define log_trace()      _log_trace(__FILE__, __LINE__)
#else
        #define log_debug(...)
        #define log_info(...)
        #define log_warning(...)
        #define log_error(...)
        #define log_trace()
#endif

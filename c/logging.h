#ifndef LOGGING_H
#define LOGGING_H

enum log_level {LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR};

void log_set(enum log_level level, int prefix);
void _log_debug(const char *fmt, ...);
void _log_info(const char *fmt, ...);
void _log_warning(const char *fmt, ...);
void _log_error(const char *fmt, ...);

#if defined LOGGING
    #define log_debug(...) _log_debug(__VA_ARGS__)
    #define log_info(...) _log_info(__VA_ARGS__)
    #define log_warning(...) _log_warning(__VA_ARGS__)
    #define log_error(...) _log_error(__VA_ARGS__)
#else
    #define log_debug(...)
    #define log_info(...)
    #define log_warning(...)
    #define log_error(...)
#endif

#endif

#ifndef LOGGING_H
#define LOGGING_H

enum log_level {LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR};

void log_set(enum log_level level, int prefix);
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warning(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif

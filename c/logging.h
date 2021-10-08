#ifndef LOGGING_H
#define LOGGING_H

enum log_level {DEBUG, INFO, WARNING, ERROR};

void log_set(enum log_level level, int prefix);
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warning(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif

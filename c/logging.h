#ifndef LOGGING_H
#define LOGGING_H

enum loglevel {DEBUG, INFO, WARNING, ERROR};

void set_loglevel(enum loglevel level);
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warning(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif

#include <stdio.h>
#include <syslog.h>
#include <stdbool.h>
#include <stdarg.h>

#include "common.h"
#include "logging.h"

static bool is_open = false;

static void log_open(void)
{
    const char *ident = cmd_args->cmd_name;
    int option = LOG_CONS | LOG_PID;
    int facility = LOG_DAEMON;

    if (!is_open) {
        openlog(ident, option, facility);

        is_open = true;
    }
}

static void log_close(void)
{
    closelog();

    is_open = false;
}

static void dump_syslog(int level, const char *msg)
{
    syslog(level, msg);

    if (cmd_args->debug) {
        char level_str[12];

        switch (level) {
        case LOG_INFO:
            sprintf(level_str, "INFO");
            break;
        case LOG_DEBUG:
            sprintf(level_str, "DEBUG");
            break;
        case LOG_WARNING:
            sprintf(level_str, "WARNING");
            break;
        case LOG_ERR:
            sprintf(level_str, "ERROR");
            break;
        default:
            sprintf(level_str, "(unknown)");
            break;
        }
        printf("%s: %s\n", level_str, msg);
    }
}

static void log_info(const char *fmt, ...)
{
    log_open();

    char msg[1024];

    va_list ap;
    va_start(ap, fmt);
    vsprintf(msg, fmt, ap);
    va_end(ap);

    dump_syslog(LOG_INFO, msg);
}

static void log_debug(const char *fmt, ...)
{
    log_open();

    char msg[1024];

    va_list ap;
    va_start(ap, fmt);
    vsprintf(msg, fmt, ap);
    va_end(ap);

    dump_syslog(LOG_DEBUG, msg);
}

static void log_warn(const char *fmt, ...)
{
    log_open();

    char msg[1024];

    va_list ap;
    va_start(ap, fmt);
    vsprintf(msg, fmt, ap);
    va_end(ap);

    dump_syslog(LOG_WARNING, msg);
}

static void log_error(const char *fmt, ...)
{
    log_open();

    char msg[1024];

    va_list ap;
    va_start(ap, fmt);
    vsprintf(msg, fmt, ap);
    va_end(ap);

    dump_syslog(LOG_ERR, msg);
}

klog_t klog = {
    .open = log_open,
    .close = log_close,
    .info = log_info,
    .debug = log_debug,
    .warn = log_warn,
    .error = log_error,
};

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <syslog.h>

#include "common.h"
#include "logger.h"
#include "input-args.h"

static bool is_open = false;

static void log_do(int priority, const char *fmt, va_list ap);

static cmd_args_t cmd_args;

static void log_open(cmd_args_t args)
{
    cmd_args = args;

    if (!is_open && cmd_args.prog_name) {
        int option = LOG_CONS | LOG_PID;
        int facility = LOG_DAEMON;

        int logmask = LOG_MASK(LOG_ERR)
                    | LOG_MASK(LOG_WARNING)
                    | LOG_MASK(LOG_INFO)
                    | LOG_MASK(LOG_DEBUG);

        setlogmask(logmask);

        openlog(cmd_args.prog_name, option, facility);

        is_open = true;
    }
}

static void log_close(void)
{
    closelog();

    is_open = false;
}

static void log_info(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_do(LOG_INFO, fmt, ap);
    va_end(ap);
}

static void log_debug(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_do(LOG_DEBUG, fmt, ap);
    va_end(ap);
}

static void log_notice(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_do(LOG_NOTICE, fmt, ap);
    va_end(ap);
}

static void log_warn(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_do(LOG_WARNING, fmt, ap);
    va_end(ap);
}

static void log_error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_do(LOG_ERR, fmt, ap);
    va_end(ap);
}

static void log_do(int priority, const char *fmt, va_list ap)
{
    char buf[MAX_LOG_LEN];

    if (!is_open) {
        return;
    }

    vsnprintf(buf, MAX_LOG_LEN - 1, fmt, ap);

    syslog(priority, "%s", buf);

    if (cmd_args.debug) {
        char level_str[12];

        switch (priority) {
        case LOG_INFO:
            /* informational message */
            sprintf(level_str, "INFO");
            break;
        case LOG_DEBUG:
            /* debug-level message */
            sprintf(level_str, "DEBUG");
            break;
        case LOG_NOTICE:
            /* normal, but significant, condition */
            sprintf(level_str, "NOTICE");
            break;
        case LOG_WARNING:
            /* warning conditions */
            sprintf(level_str, "WARNING");
            break;
        case LOG_ERR:
            /* error conditions */
            sprintf(level_str, "ERROR");
            break;
        default:
            /* LOG_EMERG, LOG_ALERT, LOG_CRIT */
            sprintf(level_str, "(unknown)");
            break;
        }
        printf("%s: %s\n", level_str, buf);
    }
}

logger_t logger = {
    .open = log_open,
    .close = log_close,
    .info = log_info,
    .debug = log_debug,
    .notice = log_notice,
    .warn = log_warn,
    .error = log_error,
};

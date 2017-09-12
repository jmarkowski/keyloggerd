#include <stdio.h>
#include <syslog.h>
#include <stdbool.h>

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

static void log_info(const char *msg)
{
    log_open();

    syslog(LOG_INFO, msg);
}

static void log_debug(const char *msg)
{
    log_open();

    syslog(LOG_DEBUG, msg);

    if (cmd_args->debug) {
        printf("%s\n", msg);
    }
}

static void log_warn(const char *msg)
{
    log_open();

    syslog(LOG_WARNING, msg);
}

static void log_error(const char *msg)
{
    log_open();

    syslog(LOG_ERR, msg);
}

klog_t klog = {
    .open = log_open,
    .close = log_close,
    .info = log_info,
    .debug = log_debug,
    .warn = log_warn,
    .error = log_error,
};

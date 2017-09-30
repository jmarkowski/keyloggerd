#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"

static void err_do(const char *fmt, va_list ap);

void err_quit(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_do(fmt, ap);
    va_end(ap);

    exit(1);
}

static void err_do(const char *fmt, va_list ap)
{
    char buf[MAX_LOG_LEN];

    vsnprintf(buf, MAX_LOG_LEN - 1, fmt, ap);

    klog.error(buf);

    strcat(buf, "\n");
    fflush(stdout); /* In case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(NULL); /* flush all stdio output streams */
}

#include <stdio.h>
#include <stdlib.h>     /* for exit  */
#include <string.h>     /* for strerror */
#include <sys/stat.h>   /* for mode_t */
#include <fcntl.h>      /* for open */
#include <errno.h>      /* for errno */
#include <unistd.h>     /* for getpid */

#include "common.h"
#include "error.h"

static void daemonize(void)
{
    klog.debug("Starting keylogger daemon...");
    klog.debug(cmd_args->cmd_name);
}

/* @todo set to /var/run/keyloggerd.pid when super user support is added */
#define LOCKFILE "keyloggerd.pid"

/**
 * Ensure that only one copy of the daemon is running
 */
static int already_running(void)
{
    int fd;

    int oflag = O_RDWR | O_CREAT | O_TRUNC;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fd = open(LOCKFILE, oflag, mode);

    if (fd == ERROR) {
        err_quit("Failed to open %s: %s", LOCKFILE, strerror(errno));
    }

    struct flock lockp = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_CUR,
        .l_start = 0,
        .l_len = 16,
    };

    if (fcntl(fd, F_SETLK, &lockp) == ERROR) {
        err_quit("Failed to lock %s: %s", LOCKFILE, strerror(errno));
    }

    char buf[16];
    sprintf(buf, "%ld", (long) getpid());
    write(fd, buf, strlen(buf)+1);

    return OK;
}

int main(int argc, char *argv[])
{
    cmd_args = parse_args(argc, argv);

    if (cmd_args) {
        daemonize();
    }

    if (already_running()) {
        err_quit("Daemon is already running");
    }

    exit(0);
}

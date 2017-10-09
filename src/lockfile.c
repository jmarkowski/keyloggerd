#include <errno.h>
#include <fcntl.h>      /* for open */
#include <stdio.h>      /* for remove */
#include <stdlib.h>     /* for malloc */
#include <string.h>     /* for strerror */
#include <sys/stat.h>   /* for mode_t */
#include <unistd.h>     /* for getpid */

#include "common.h"
#include "lockfile.h"
#include "logger.h"

/* Arguments same order as lseek */
#define write_lock(fd, offset, whence, len) \
    lock_do(fd, F_SETLK, F_WRLCK, (offset), (whence), (len))
#define release_lock(fd, offset, whence, len) \
    lock_do(fd, F_SETLK, F_UNLCK, (offset), (whence), (len))

static int lock_do(int fd, int cmd,
                   short type, off_t offset, short whence, off_t len)
{
    struct flock fl;

    fl.l_type = type;       /* F_RDLCK, F_WRLCK, F_UNLCK */
    fl.l_whence = whence;   /* SEEK_SET, SEEK_CUR, SEEK_END */
    fl.l_start = offset;    /* byte offset, relative to l_whence */
    fl.l_len = len;         /* # bytes (0 means to EOF) */

    return fcntl(fd, cmd, &fl);
}

static int lockfile_lock(lockfile_t *lf)
{
    if (!lf) {
        return ERROR;
    }

    int oflag = O_RDWR | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    lf->fd = open(lf->file, oflag, mode);

    if (lf->fd == ERROR) {
        logger.error("Failed to open %s: %s",
                     lf->file, strerror(errno));
        return ERROR;
    }

    if (write_lock(lf->fd, 0, SEEK_SET, 0) == ERROR) {
        /* Lock entire file for writing */
        logger.error("Failed to lock %s: %s",
                     lf->file, strerror(errno));
        return ERROR;
    }

    char buf[16] = { '\0' };
    sprintf(buf, "%ld\n", (long) getpid());

    lseek(lf->fd, 0, SEEK_SET); /* go to beginning */
    write(lf->fd, buf, strlen(buf));

    logger.info("Obtained PID file lock");

    return OK;
}

static int lockfile_unlock(lockfile_t *lf)
{
    if (!lf || lf->fd == -1) {
        return ERROR;
    }

    if (release_lock(lf->fd, 0, SEEK_SET, 0) == ERROR) {
        /* Unlock entire file */
        logger.error("Failed to unlock %s: %s",
                     lf->file, strerror(errno));
        return ERROR;
    }

    close(lf->fd);

    logger.info("Released PID file lock");

    lf->fd = -1;

    if (remove(lf->file) == ERROR) {
        logger.warn("Failed to remove %s: %s",
                    lf->file, strerror(errno));
        return ERROR;
    }

    return OK;
}

lockfile_t * create_lockfile(const char * const file)
{
    lockfile_t *lf;

    lf = (lockfile_t *) malloc(sizeof(lockfile_t));

    lf->lock = lockfile_lock;
    lf->unlock = lockfile_unlock;
    lf->file = file;
    lf->fd = -1;

    return lf;
}

void destroy_lockfile(lockfile_t *lf)
{
    free(lf);
}

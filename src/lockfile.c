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

static int lockfile_lock(const lockfile_t *lf)
{
    if (!lf) {
        return ERROR;
    }

    int oflag = O_RDWR | O_CREAT | O_TRUNC;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int fd = open(lf->file, oflag, mode);

    if (fd == ERROR) {
        logger.error("Failed to open %s: %s",
                     lf->file, strerror(errno));
        return ERROR;
    }

    struct flock lockp = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_CUR,
        .l_start = 0,
        .l_len = 16,
    };

    if (fcntl(fd, F_SETLK, &lockp) == ERROR) {
        logger.error("Failed to lock %s: %s",
                     lf->file, strerror(errno));
        return ERROR;
    }

    char buf[16] = { '\0' };
    sprintf(buf, "%ld\n", (long) getpid());
    write(fd, buf, strlen(buf));

    close(fd);

    return OK;
}

static int lockfile_unlock(const lockfile_t *lf)
{
    if (!lf) {
        return ERROR;
    }

    if (remove(lf->file) == ERROR) {
        logger.error("Failed to remove %s: %s",
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

    return lf;
}

void destroy_lockfile(lockfile_t *lf)
{
    free(lf);
}

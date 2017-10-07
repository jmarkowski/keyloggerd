#ifndef LOCK_FILE_H
#define LOCK_FILE_H

typedef struct lockfile {
    int (*lock)(const struct lockfile *lf);
    int (*unlock)(const struct lockfile *lf);
    const char *file;
} lockfile_t;

extern lockfile_t * create_lockfile(const char *file);
extern void destroy_lockfile(lockfile_t *lf);

#endif

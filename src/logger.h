#ifndef LOGGING_H
#define LOGGING_H

#include "parse-args.h"

#define MAX_LOG_LEN 1024

typedef struct {
    void (*open)(cmd_args_t args);
    void (*close)(void);
    void (*info)(const char *fmt, ...);
    void (*debug)(const char *fmt, ...);
    void (*warn)(const char *fmt, ...);
    void (*error)(const char *fmt, ...);
} logger_t;

extern logger_t logger;

#endif

#ifndef LOGGING_H
#define LOGGING_H

#define MAX_LOG_LEN 1024

typedef struct {
    void (*open)(const char *ident);
    void (*close)(void);
    void (*info)(const char *fmt, ...);
    void (*debug)(const char *fmt, ...);
    void (*warn)(const char *fmt, ...);
    void (*error)(const char *fmt, ...);
} logger_t;

extern logger_t logger;

#endif

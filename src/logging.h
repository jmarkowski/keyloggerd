#ifndef LOGGING_H
#define LOGGING_H

#define MAX_LOG_LEN 1024

typedef struct {
    void (*open)(void);
    void (*close)(void);
    void (*info)(const char *fmt, ...);
    void (*debug)(const char *fmt, ...);
    void (*warn)(const char *fmt, ...);
    void (*error)(const char *fmt, ...);
} klog_t;

extern klog_t klog;

#endif

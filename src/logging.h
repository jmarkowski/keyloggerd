#ifndef LOGGING_H
#define LOGGING_H

typedef struct {
    void (*open)(void);
    void (*close)(void);
    void (*info)(const char *msg);
    void (*debug)(const char *msg);
    void (*warn)(const char *msg);
    void (*error)(const char *msg);
} klog_t;

extern klog_t klog;

#endif

#ifndef KEY_LOG_H
#define KEY_LOG_H

#include <linux/input.h>
#include <stdbool.h>

#include "common.h"
#include "input-args.h"

enum {
    KEY_LOG_FLAG_APPEND = BIT(0)
};

typedef struct keylog keylog_t;
typedef struct keyseq keyseq_t;

struct keyseq {
    unsigned short *keys;
    unsigned short size;
    unsigned short index;
    void (*callback)(keylog_t *kl);
};

struct keylog {
    bool logging_enabled;

    int (*open)(keylog_t *kl);
    int (*close)(keylog_t *kl);
    void (*process_event)(keylog_t *kl, struct input_event e);
    void (*install_seq)(keylog_t *kl, keyseq_t ks);

    void (*pause)(keylog_t *kl);
    void (*resume)(keylog_t *kl);

    void *priv;
};

extern keylog_t *create_keylog(const cmd_args_t cmd_args);
extern void destroy_keylog(keylog_t *kl);

#endif

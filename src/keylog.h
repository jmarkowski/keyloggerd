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

typedef struct keylog {
    bool logging_enabled;

    int (*open)(struct keylog *kl);
    int (*close)(struct keylog *kl);
    void (*process_event)(struct keylog *kl, struct input_event e);
    void (*install_seq)(struct keylog *kl, struct keyseq ks);

    void (*pause)(struct keylog *kl);
    void (*resume)(struct keylog *kl);

    void *priv;
} keylog_t;

extern keylog_t *create_keylog(const cmd_args_t cmd_args);
extern void destroy_keylog(keylog_t *kl);

#endif

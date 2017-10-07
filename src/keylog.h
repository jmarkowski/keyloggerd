#ifndef KEY_LOG_H
#define KEY_LOG_H

#include <linux/input.h>
#include <stdbool.h>

#include "common.h"
#include "input-args.h"

enum {
    KEY_LOG_FLAG_APPEND = BIT(0)
};

typedef struct keylog {
    int (*open)(struct keylog *kl);
    int (*close)(struct keylog *kl);
    void (*log)(struct keylog *kl, struct input_event e);

    void *priv;
} keylog_t;

extern keylog_t *create_keylog(const cmd_args_t cmd_args);
extern void destroy_keylog(keylog_t *kl);

#endif

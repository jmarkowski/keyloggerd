#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include <stdbool.h>
#include <stdlib.h>     /* for mode_t */

#include "common.h"

#define MAX_PROG_NAME 12
#define MAX_DEVICE_PATH 100

typedef struct {
    bool debug;
    char prog_name[MAX_PROG_NAME];
    char keyboard_device[MAX_DEVICE_PATH];

    struct {
        char filename[KEY_LOG_LEN];
        char backspace;
        mode_t mode;
        int flags;
    } keylog;
} cmd_args_t;

extern cmd_args_t parse_args(int argc, char *argv[]);

#endif

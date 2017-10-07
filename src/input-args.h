#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include <stdbool.h>
#include <stdlib.h>     /* for mode_t */

#define MAX_PROG_NAME 12
#define MAX_DEVICE_PATH 100

typedef struct {
    char prog_name[MAX_PROG_NAME];
    char keyboard_device[MAX_DEVICE_PATH];
    bool debug;
    mode_t keylog_mode;
} cmd_args_t;

extern cmd_args_t parse_args(int argc, char *argv[]);

#endif

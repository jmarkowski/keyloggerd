#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include <stdbool.h>

#define MAX_PROG_NAME 12

typedef struct {
    char prog_name[MAX_PROG_NAME];
    bool debug;
} cmd_args_t;

extern cmd_args_t parse_args(int argc, char *argv[]);

#endif

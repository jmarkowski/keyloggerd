#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include <stdbool.h>

typedef struct {
    char *cmd_name;
    bool debug;
} cmd_args_t;

extern cmd_args_t *parse_args(int argc, char *argv[]);

#endif

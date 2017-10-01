#include <stdbool.h>    /* for bool */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* for strrchr */

#include "input-args.h"

static const char usage_str[] =
    "keyloggerd [-h | --help] [--debug]";

#define is_equal(a, b) (!strcmp(a, b))

cmd_args_t parse_args(int argc, char *argv[])
{
    cmd_args_t cmd_args = {};
    char *prog_name;

    if ((prog_name = strrchr(argv[0], '/')) == NULL) {
        prog_name = argv[0];
    } else {
        prog_name++;
    }

    for (int k = 1; k < argc; k++) {
        const char *arg = argv[k];

        if (is_equal(arg, "--debug")) {
            cmd_args.debug = true;
        } else if (is_equal(arg, "--help") || is_equal(arg, "-h")) {
            printf("%s\n", usage_str);
            exit(0);
        } else {
            printf("Unknown option: %s\n", argv[k]);
            printf("%s\n", usage_str);
            exit(1);
        }
    }

    strncpy(cmd_args.prog_name, prog_name, MAX_PROG_NAME);

    return cmd_args;
}

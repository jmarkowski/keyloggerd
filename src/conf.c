#include <linux/input.h>
#include <stdbool.h>    /* for bool */
#include <stdio.h>      /* for fopen */
#include <string.h>     /* for strcmp */

#include "common.h"
#include "conf.h"
#include "keylog.h"

#define CONF_PATH "keyloggerd.conf"

#define is_equal(a, b) (!strcmp(a, b))

#define SECTION_LEN 64

#define LINE_LEN 512

static char section[SECTION_LEN];

/*
 * Perform an inplace trim of any white space on the left and right sides
 */
static void str_trim(char *str)
{
    int k;
    char buf[LINE_LEN];
    char *s; /* start */
    char *e; /* end */

    strncpy(buf, str, LINE_LEN);
    s = buf;

    /* left trim */
    for (k = 0; str[k] != '\0'; k++) {
        if (str[k] != ' ' && str[k] != '\t' && str[k] != '\n') {
            break;
        }
    }

    s += k;

    /* right trim */
    for (k = strlen(str) - 1; k >= 0; k--) {
        if (str[k] != ' ' && str[k] != '\t' && str[k] != '\n') {
            break;
        }
    }

    if (k != -1) {
        e = &buf[k];

        memset(str, 0, strlen(str));
        strncpy(str, s, e - s + 1);
    }
}

static unsigned key_str2enum(const char * const key_str)
{
    if (is_equal(key_str, "<ESC>")) {
        return KEY_ESC;
    } else if (is_equal(key_str, "<LEFTSHIFT>")) {
        return KEY_LEFTSHIFT;
    } else if (is_equal(key_str, "<RIGHTSHIFT>")) {
        return KEY_RIGHTSHIFT;
    } else if (is_equal(key_str, "<LEFTCTRL>")) {
        return KEY_LEFTCTRL;
    } else if (is_equal(key_str, "<RIGHTCTRL>")) {
        return KEY_RIGHTCTRL;
    } else if (is_equal(key_str, "<LEFTALT>")) {
        return KEY_LEFTALT;
    } else if (is_equal(key_str, "<RIGHTALT>")) {
        return KEY_RIGHTALT;
    } else if (is_equal(key_str, "<LEFTMETA>")) {
        return KEY_LEFTMETA;
    } else if (is_equal(key_str, "<RIGHTMETA>")) {
        return KEY_RIGHTMETA;
    } else if (is_equal(key_str, "<F1>")) {
        return KEY_F1;
    } else if (is_equal(key_str, "<F2>")) {
        return KEY_F2;
    } else if (is_equal(key_str, "<F3>")) {
        return KEY_F3;
    } else if (is_equal(key_str, "<F4>")) {
        return KEY_F4;
    } else if (is_equal(key_str, "<F5>")) {
        return KEY_F5;
    } else if (is_equal(key_str, "<F6>")) {
        return KEY_F6;
    } else if (is_equal(key_str, "<F7>")) {
        return KEY_F7;
    } else if (is_equal(key_str, "<F8>")) {
        return KEY_F8;
    } else if (is_equal(key_str, "<F9>")) {
        return KEY_F9;
    } else if (is_equal(key_str, "<F10>")) {
        return KEY_F10;
    } else if (is_equal(key_str, "<F11>")) {
        return KEY_F11;
    } else if (is_equal(key_str, "<F12>")) {
        return KEY_F12;
    } else {
        return 0;
    }
}

static int set_seq(uarray_t *seq, const char * const strseq)
{
    const char *delim = ",";
    const char *tok;
    char tok_trim[LINE_LEN];
    unsigned key;
    char buf_copy[LINE_LEN];

    strncpy(buf_copy, strseq, LINE_LEN);

    tok = strtok(buf_copy, delim);
    strncpy(tok_trim, tok, LINE_LEN);

    str_trim(tok_trim);

    seq->size = 0;
    seq->el = malloc((seq->size + 1) * sizeof(unsigned));

    if ((key = key_str2enum(tok_trim)) == 0) {
        return ERROR;
    }
    seq->el[seq->size++] = key;

    while ((tok = strtok(NULL, delim)) != NULL) {
        strncpy(tok_trim, tok, LINE_LEN);
        str_trim(tok_trim);

        seq->el = realloc(seq->el, (seq->size + 1) * sizeof(unsigned));

        if ((key = key_str2enum(tok_trim)) == 0) {
            return ERROR;
        }
        seq->el[seq->size++] = key;
    }

    return OK;
}

static bool is_section(char *sect)
{
    bool is_section = false;;
    char *a = strchr(sect, '[');
    char *b = strchr(sect, ']');

    if (a && b) {
        is_section = (b - a) == (strlen(sect) - 2);
    }

    return is_section;
}

static void set_section(char *sect)
{
    char *b = strchr(sect, ']');
    size_t len = b - sect + 1;

    memset(section, 0, SECTION_LEN);
    strncpy(section, sect, len);
}

static void set_option(char *opt, cmd_args_t *args)
{
    char key[LINE_LEN];
    char val[LINE_LEN];
    const char *delim = "=";

    char buf_copy[LINE_LEN];
    char *tok;

    strncpy(buf_copy, opt, LINE_LEN);

    tok = strtok(buf_copy, delim);
    strncpy(key, tok, strlen(tok) + 1);
    tok = strtok(NULL, delim);
    strncpy(val, tok, strlen(tok) + 1);

    str_trim(key);
    str_trim(val);

    if (is_equal(section, "[main]")) {
        if (is_equal(key, "keyboard_device")) {
            strncpy(args->keyboard_device, val, MAX_DEVICE_PATH);
        } else {
            printf("Unknown [main] entry: '%s'\n", key);
        }
    } else if (is_equal(section, "[log]")) {
        if (is_equal(key, "mode")) {
            args->keylog.mode = str2mode(val);
        } else if (is_equal(key, "append")) {
            if (is_equal(val, "true")) {
                args->keylog.flags |= KEY_LOG_FLAG_APPEND;
            } else if (is_equal(val, "false")) {
                args->keylog.flags &= ~KEY_LOG_FLAG_APPEND;
            } else {
                printf("Invalid value for key '%s': '%s'\n", key, val);
            }
        } else if (is_equal(key, "backspace_char")) {
            args->keylog.backspace_char = val[0];
        } else if (is_equal(key, "path")) {
            strncpy(args->keylog.path, val, LOG_PATH_LEN);
        } else {
            printf("Unknown [log] entry: '%s'\n", key);
        }
    } else if (is_equal(section, "[sequence]")) {
        if (is_equal(key, "kill")) {
            if (set_seq(&args->seq.kill, val) == ERROR) {
                printf("Failed to install '%s = %s'\n", key, val);
            }
        } else if (is_equal(key, "pause_resume")) {
            if (set_seq(&args->seq.pause_resume, val) == ERROR) {
                printf("Failed to install '%s = %s'\n", key, val);
            }
        } else {
            printf("Unknown [sequence] entry: '%s'\n", key);
        }
    } else {
        printf("Unknown section: '%s'\n", section);
    }
}

cmd_args_t read_conf(void)
{
    cmd_args_t conf_args = { 0 };

    /* buffered I/O */
    FILE *fp;
    char buf[LINE_LEN];

    fp = fopen(CONF_PATH, "r");

    if (fp) {
        while (fgets(buf, LINE_LEN, fp) != NULL) {
            if (is_section(buf)) {
                set_section(buf);
            } else if (strchr(buf, '=') != NULL && buf[0] != '#') {
                /*
                 * If the sequence isn't a comment, and it has an '=' (key,
                 * value pair)
                 */
                set_option(buf, &conf_args);
            }
        }
    }

    return conf_args;
}

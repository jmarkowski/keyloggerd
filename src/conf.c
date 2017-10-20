#include <stdbool.h>    /* for bool */
#include <stdio.h>      /* for fopen */
#include <string.h>     /* for strcmp */

#include "conf.h"
#include "input-args.h"
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

    e = &buf[k];

    memset(str, 0, LINE_LEN);
    strncpy(str, s, e - s + 1);
}

static bool is_section(char *sect)
{
    bool is_section = false;;
    char *a = strchr(sect, '[');
    char *b = strchr(sect, ']');

    if (a && b) {
        is_section = (b - a == strlen(sect) - 2);
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
        } else if (is_equal(key, "path")) {
            strncpy(args->keylog.path, val, LOG_PATH_LEN);
        } else {
            printf("Unknown [log] entry: '%s'\n", key);
        }
    } else if (is_equal(section, "[sequence]")) {
#if 0
        if (is_equal(key, "kill")) {
            printf("kill is set to %s\n", val);
        } else if (is_equal(key, "pause_resume")) {
            printf("pause_resume is set to %s\n", val);
        } else {
            printf("Unknown [sequence] entry: '%s'\n", key);
        }
#endif
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

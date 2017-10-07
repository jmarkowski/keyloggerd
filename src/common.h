#ifndef COMMON_H
#define COMMON_H

/* Utility defines */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define BIT(shift) (1 << (shift))

#define OK 0
#define ERROR -1

/* Shared defines */
#define KEY_LOG_LEN 128

#endif

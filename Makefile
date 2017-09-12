# Define V=1 for a verbose compile.
ifneq ($(V),1)
QUIET_CC = @echo '   ' CC $@;
QUIET_LD = @echo '   ' LD $@;
QUIET_SUBDIR = @echo '   ' SUBDIR $@;
export V
endif

CC = gcc
CFLAGS  = -c -Wall -O0 -ggdb -std=c11

SRCS += main.c
SRCS += logging.c
SRCS += parse-args.c

PROG = keyloggerd

VPATH = src

OBJ_DIR = obj
OBJS = $(addprefix $(OBJ_DIR)/,$(SRCS:.c=.o))

all: $(PROG)

$(PROG): $(OBJS)
	$(QUIET_LD)$(CC) -o $(PROG) $^

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(QUIET_CC)$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR):
	$(QUIET_SUBDIR)mkdir -p $@

print-%:
# Debug target used for dumping makefile variables.
# usage: make print-VAR
	@echo '$*=$($*)'

clean:
	-rm -rf $(OBJ_DIR)
	-rm -f $(PROG)

.PHONY: all clean

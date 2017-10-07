PROG = keyloggerd

CC = gcc
CFLAGS  = -c -Wall -O0 -ggdb -std=gnu11

CPPFLAGS = -DDEBUG

C_FILES += main.c
C_FILES += error.c
C_FILES += input-args.c
C_FILES += keyloggerd.c
C_FILES += lockfile.c
C_FILES += logger.c

SRC_DIR = src
OBJ_DIR = obj

SRCS = $(addprefix $(SRC_DIR)/,$(C_FILES))
OBJS = $(addprefix $(OBJ_DIR)/,$(C_FILES:.c=.o))

# Define V=1 for a verbose compile.
ifneq ($(V),1)
QUIET_CC = @echo '   ' CC $@;
QUIET_LD = @echo '   ' LD $@;
QUIET_SUBDIR = @echo '   ' SUBDIR $@;
export V
endif

all: $(PROG)

$(PROG): $(OBJS)
	$(QUIET_LD)$(CC) -o $(PROG) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(QUIET_CC)$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@

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

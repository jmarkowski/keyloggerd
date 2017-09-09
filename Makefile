CC = gcc
CFLAGS = -Wall -O0 -ggdb -std=c11

PROG = keyloggerd

all: src/main.c
	$(CC) $(CFLAGS) $< -o $(PROG)

print-%:
# Debug target used for dumping makefile variables.
# usage: make print-VAR
	@echo '$*=$($*)'

clean:
	-rm -f *.o
	-rm $(PROG)

.PHONY: all clean

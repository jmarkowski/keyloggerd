CC = gcc
CFLAGS = -Wall -O0 -ggdb -std=c11

PROG = keyloggerd

all: src/main.c
	$(CC) $(CFLAGS) $< -o $(PROG)

clean:
	-rm -f *.o
	-rm $(PROG)

.PHONY: all clean

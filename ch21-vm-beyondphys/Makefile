
.PHONY: all clean

CC=gcc
CFLAGS=-Wall -O
SRC=$(wildcard *.c)
BINARIES=$(subst .c,,$(SRC))

all: $(BINARIES)

$(word 1, $(BINARIES)): %: %.c
	gcc -o $@ $< $(CFLAGS)

clean:
	rm -rf $(BINARIES) *.o *.out


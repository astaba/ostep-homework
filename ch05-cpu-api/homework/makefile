# cpu-api/homework/makefile
.SUFFIXES :
.SUFFIXES: .c .out
.PHONY: all clean

CC  = clang
STD ?= -std=gnu17
WAR ?= -Wall
OPT ?= -O3 -march=native -fdata-sections -ffunction-sections
CFLAGS = ${WAR}

SOURCES = $(wildcard *.c)
BINARIES = $(subst .c,.out,$(SOURCES))

all: $(BINARIES)

$(BINARIES): %.out: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -fv *.out boo.txt list.txt

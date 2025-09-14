# ostep-homework/ch05-cpu-api/makefile
# Created on: Sun Sep 14 11:53:44 +01 2025

.PHONY: all clean
.DELETE_ON_ERROR:

CC     := gcc
# STD    := -std=gnu17
WAR    := -Wall
# OPT    := -O3 -march=native -fdata-sections -ffunction-sections
CFLAGS := ${WAR}

SOURCES  = $(wildcard *.c)
BINARIES = $(subst .c,.out,$(SOURCES))

all: $(BINARIES)

$(BINARIES): %.out: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -fv *.out

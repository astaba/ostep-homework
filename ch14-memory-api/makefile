# c_prog/ostep-homework/ch13-address-spaces/makefile
# Created on: Mon Oct 20 23:14:42 +01 2025

.PHONY: all clean
.DELETE_ON_ERROR:

# ==============================================================================
CC       := gcc
CFLAGS   := -Wall
DBGFLAGS := -g3 -O0 -DDEBUG
# ==============================================================================
SOURCES  := $(wildcard *.c)
BINARIES := $(subst .c,.out,$(SOURCES))
GINARIES := $(subst .c,.dbg.out,$(SOURCES))
# ==============================================================================
all: $(BINARIES)

# ==============================================================================
$(BINARIES): %.out: %.c
	$(CC) $(CFLAGS) $< -o $@

$(GINARIES): %.dbg.out: %.c
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@
# ==============================================================================
clean:
	rm -fv *.out

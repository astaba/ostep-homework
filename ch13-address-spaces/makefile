# c_prog/ostep-homework/ch13-address-spaces/makefile
# Created on: Mon Oct 20 23:14:42 +01 2025

.PHONY: all clean
.DELETE_ON_ERROR:

# ==============================================================================
CC       := gcc
CFLAGS   := -Wall -Werror
# Optional debug flags for development
# DBGFLAGS := -g3 -O0 -DDEBUG
# ==============================================================================
# SOURCES  := scllaps.c cswlaps.c
SOURCES  := $(wildcard *.c)
BINARIES := $(subst .c,.out,$(SOURCES))
# ==============================================================================
all: $(BINARIES)

# ==============================================================================
$(BINARIES): %.out: %.c
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

# ==============================================================================
clean:
	rm -fv *.out

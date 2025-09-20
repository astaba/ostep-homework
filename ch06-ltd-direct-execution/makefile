# ostep-homework/ch06-ltd-direct-execution/makefile#

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

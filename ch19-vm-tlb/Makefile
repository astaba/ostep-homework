# ostep-homework/ch19-vm-tlb/Makefile
# Created on: Fri Dec  5 08:45:16 +01 2025

.PHONY: test clean all
.DELETE_ON_ERROR:

# ==============================================================================
CC       := gcc
OPFLAGS  := -fno-tree-vectorize -fno-unroll-loops
# Optional debug flags for development
DBGFLAGS := -g3 -O0 -DDEBUG -fno-tree-vectorize -fno-unroll-loops
CFLAGS   := -Wall -Werror
CFLAGS   += $(OPFLAGS)
# CFLAGS  += $(DBGFLAGS)
# ==============================================================================
SOURCES  := $(wildcard *.c)
BINARIES := $(subst .c,,$(SOURCES))
# ==============================================================================
all: $(BINARIES)

# ==============================================================================
$(BINARIES): %: %.c
	$(CC) $(CFLAGS) $< -o $@

# ==============================================================================
clean:
	rm -fv $(BINARIES) *.out

# ostep-homework/ch06-ltd-direct-execution/syslaps64/makefile
# Created on: Fri Sep 19 06:59:59 +01 2025

.PHONY: all clean
.DELETE_ON_ERROR:

# ==============================================================================
CC       := gcc
CFLAGS   := -Wall -Werror
# Optional debug flags for development
DBGFLAGS := -g3 -O0 -DDEBUG
# ASFLAGS := -f elf64 -g -F DWARF
ASFLAGS := -f elf64
# ==============================================================================
SOURCES  := syslaps-v2.c timestamp64.asm
MODULES  := syslaps64.o timestamp64.o
BINARY   := syslaps-v2.out
# ==============================================================================
all: $(BINARY)

# ==============================================================================
$(BINARY): $(word 1,$(SOURCES)) $(word 2,$(MODULES))
	$(CC) $(CFLAGS) $(DBGFLAGS) $^ -o $@

# ==============================================================================
$(word 2,$(MODULES)): $(word 2,$(SOURCES))
	nasm $(ASFLAGS) $< -o $@ -l $(basename $(@F)).lst
#
# ==============================================================================
clean:
	rm -fv *.o *.lst *.out

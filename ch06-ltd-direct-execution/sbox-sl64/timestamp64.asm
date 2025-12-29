; ostep-homework/ch06-ltd-direct-execution/sbox-sl64/timestamp64.asm
; Created on: Tue Dec 16 20:04:50 +01 2025
;; =============================================================================
; Timing Routines
; =============================================================================
; This file contains assembly routines for high-precision timing using the
; Time-Stamp Counter (TSC). The routines are designed to be called from C.
;
; The TSC is a 64-bit register that counts the number of clock cycles since
; the last CPU reset. It provides a much more granular measurement than
; system timers.

section .text

global rdtsc_call
global repeat_read_syscall

rdtsc_call:
	rdtsc
	shl	RDX, 32
	or	RAX, RDX
	ret

repeat_read_syscall:
	mov R8D, EDI
.loop_read:
	xor RAX, RAX ; sys_read ID: 0
	xor RDI, RDI ; read source: stdin == 0
	xor RSI, RSI ; read destination: NULL (irrelevant for reading 0 byte)
	xor RDX, RDX ; read size: 0 byte
	syscall
	dec R8D
	jnz .loop_read
	ret

%ifidn __OUTPUT_FORMAT__,elf64
	section .note.GNU-stack noalloc noexec nowrite progbits
%endif

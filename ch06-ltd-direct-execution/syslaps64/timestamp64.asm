; ostep-homework/ch06-ltd-direct-execution/timestamp64.asm
; Created on: Fri Sep 19 04:34:29 +01 2025
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
	; Make the functions visible to the linker.
	global repeat_read_syscall
	global rdtsc_start

; =============================================================================
; unsigned long long rdtsc_start(void)
; -----------------------------------------------------------------------------
; Reads the 64-bit Time-Stamp Counter (TSC) and returns the value.
; Returns:
;   The TSC value in RAX:RDX
; =============================================================================
rdtsc_start:
	; The RDTSC instruction reads the 64-bit TSC into two registers:
	; the high-order 32 bits into RDX and the low-order 32 bits into RAX.
	rdtsc
	; To get a single 64-bit value, we shift the high-order part (RDX)
	; 32 bits to the left to make room for the low-order part.
	shl rdx, 32
	; We then bitwise-OR the two registers to combine them into RAX,
	; creating a single 64-bit return value. This handles the
	; concatenation of RDX and RAX into a single register.
	or rax, rdx
	ret

; =============================================================================
; void repeat_read_syscall(int num_loops)
; -----------------------------------------------------------------------------
; Repeatedly performs a 0-byte read() system call to measure its overhead.
; Arguments:
;   num_loops (in EDI): The number of times to repeat the syscall.
; =============================================================================
repeat_read_syscall:
	; Move the loop counter from EDI (first integer argument in x64 ABI)
	; into a general-purpose register, R8D, for use in the loop.
	mov r8d, edi
.loop_start:
	; Prepare for the 0-byte read() system call.
	; A syscall requires the syscall number in RAX and arguments in RDI, RSI, RDX.
	; We use XOR to efficiently zero out the registers.
	xor rax, rax		; syscall number for sys_read is 0
	xor rdi, rdi		; file descriptor is 0 (stdin)
	xor rsi, rsi		; buffer address is NULL for 0-byte read
	xor rdx, rdx		; count (number of bytes to read) is 0
	syscall				; Execute the kernel syscall.

	; Decrement the loop counter.
	dec r8d
	; Jump back to the start of the loop if R8D is not zero.
	jnz .loop_start

	ret

; =============================================================================
; On Linux ELF64, mark stack as non-executable (security hint).
; This directive prevents the linker from creating a potentially insecure
; executable with an executable stack. It should be used in all assembly files.
; The macro is a NASM conditional statement.
%ifidn __OUTPUT_FORMAT__,elf64
	section .note.GNU-stack noalloc noexec nowrite progbits
%endif

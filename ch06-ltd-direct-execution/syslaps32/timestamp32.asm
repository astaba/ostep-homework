; ostep-homework/ch06-ltd-direct-execution/timestamp32.asm
; Created on: Fri Sep 19 01:57:17 +01 2025
; Make sure you have gcc-multilib GNU package installed for x86_32
; ==============================================================================
; NOTE: This x86_32 version is much slower than the x86_64 version.

section .text
    ; We are using C calling conventions
    ; On x86-32, arguments are pushed onto the stack
    ; Return values: 32-bit in EAX, 64-bit in EDX:EAX
    global rdtsc_start
    global repeat_read_syscall

; ==============================================================================
rdtsc_start:
    rdtsc     ; The RDTSC instruction reads the 64-bit timestamp counter
              ; into EDX (high 32 bits) and EAX (low 32 bits).
    ret       ; In the C calling convention for 64-bit returns on 32-bit x86,
              ; the high 32 bits go in EDX and the low 32 bits in EAX.
              ; RDTSC conveniently puts the values in the right place for us.

; ==============================================================================
repeat_read_syscall:    ; Stack layout:
                        ; [esp+4] = num_loops
    push ebx            ; Save caller-saved registers that we'll use
    push esi

; ------------------------------------------------------------------------------
	; (old esi) + 4 (old ebx) + 4 (ret addr) + 4 (local arg#1)
    mov esi, [esp + 12] ; Get the number of loops from the stack
.loop_start:
    ; The read() syscall is used to measure overhead
    ; We use file descriptor 0 (stdin) and a size of 0 to ensure
    ; it returns immediately.
    ; int read(int fd, void *buf, size_t count);
    ; Arguments are passed on the stack for syscalls on Linux x86.
    ; Syscall number goes into EAX.

    ; Syscall number for read() is 3
    mov eax, 3          ; read() syscall number
    xor ebx, ebx        ; fd (stdin)
    xor ecx, ecx        ; buf (we don't need one for size 0) WARN:
    xor edx, edx        ; count (0 bytes)
    int 0x80            ; Execute syscall

    dec esi             ; Decrement the loop counter
    jnz .loop_start     ; Check if we are done
; ------------------------------------------------------------------------------
    ; Restore registers and return
    pop esi
    pop ebx
    ret

; ==============================================================================
section .note.GNU-stack noalloc noexec nowrite progbits

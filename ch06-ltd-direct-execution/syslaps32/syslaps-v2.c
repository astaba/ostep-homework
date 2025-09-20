/* ostep-homework/ch06-ltd-direct-execution/syslaps32/syslaps-v2.c */
// Created on: Fri Sep 19 02:45:28 +01 2025
// Homework 6.1: Measure the cost of a syscall.
// Build:
// nasm -f elf timestamp64.asm -o timestamp64.o
// gcc syslaps-v2.c timestamp64.o -o syslaps-v2.out

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NLOOPS 1000000

// Assembly functions
extern unsigned long long rdtsc_start();
extern void repeat_read_syscall(int);

// ------------------------------------------------------------
// Helper: measure TSC frequency by waiting ~1 second
// ------------------------------------------------------------
double measure_tsc_freq(void) {
  unsigned long long t0 = rdtsc_start();
  struct timespec ts_start, ts_end;

  clock_gettime(CLOCK_MONOTONIC, &ts_start);
  sleep(1); // wait ~1 second
  clock_gettime(CLOCK_MONOTONIC, &ts_end);
  unsigned long long t1 = rdtsc_start();

  double elapsed_sec = (ts_end.tv_sec - ts_start.tv_sec) +
                       (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

  return (double)(t1 - t0) / elapsed_sec; // cycles per second
}

// ------------------------------------------------------------
int main(void) {
  printf("Starting RDTSC-based syscall measurement...\n");

  // --- PART 1: CALIBRATE CYCLES PER SECOND ---
  printf("--------------------------------------------------\n");
  printf("1. Calibrate cycles per second\n");
  double cycles_per_sec = measure_tsc_freq();
  printf("Estimated CPU frequency: %.2f GHz\n", cycles_per_sec / 1e9);

  // --- PART 2: MEASURE SYSCALL CYCLES  ---
  printf("--------------------------------------------------\n");
  printf("2. Measuring 0-byte read() syscall cycles...\n");
  printf("Running %d calls to get an average cycles.\n", NLOOPS);
  unsigned long long start_cycles = rdtsc_start();
  repeat_read_syscall(NLOOPS);
  unsigned long long end_cycles = rdtsc_start();

  unsigned long long elapsed_syscall_cycles = end_cycles - start_cycles;
  double average_cycles = (double)elapsed_syscall_cycles / NLOOPS;

  // Convert cycles → microseconds
  double cycles_per_us = cycles_per_sec / 1e6;
  double average_us = average_cycles / cycles_per_us;

  printf("Total elapsed cycles for %d syscalls: %llu cycles\n", NLOOPS,
         elapsed_syscall_cycles);
  printf("Average cycles per syscall: %.3f cycles\n", average_cycles);
  printf("--------------------------------------------------\n");
  printf("Average time per syscall: %.3f microseconds\n", average_us);
  printf("--------------------------------------------------\n");

  return EXIT_SUCCESS;
}

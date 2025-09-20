/* ostep-homework/ch06-ltd-direct-execution/syslaps64/syslaps-v2.c */
// Created on: Fri Sep 19 02:45:28 +01 2025
// Homework 6.1: Measure the cost of a syscall.
// Build:
// nasm -f elf64 timestamp64.asm -o timestamp64.o
// gcc syslaps-v2.c timestamp64.o -o syslaps-v2.out

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Define the number of iterations for our timing loops.
// Using a large number helps to get a more stable and reliable average,
// as it smooths out any temporary fluctuations or noise from the system.
#define NLOOPS 1000000

// We must declare our assembly functions here so the C compiler
// knows their names and signatures, allowing us to call them.
extern unsigned long long rdtsc_start(void);
extern void repeat_read_syscall(unsigned int);

// ----------------------------------------------------------------------------
// Helper: Measure TSC frequency by waiting ~1 second
// ----------------------------------------------------------------------------
// This function is a critical calibration routine. It uses two different
// timers to determine the CPU's clock speed at runtime, which is necessary
// for converting raw RDTSC cycle counts into a human-readable time (e.g.,
// microseconds). We can't rely on a hardcoded frequency because modern CPUs
// have dynamic speeds (turbo boost, power saving modes, etc.).
double measure_tsc_freq(void) {
  // Read the initial value of the CPU's Time-Stamp Counter (TSC).
  // This is our high-resolution, CPU-level stopwatch.
  unsigned long long t0 = rdtsc_start();
  struct timespec ts_start, ts_end;
  // Use a standard system clock to measure an elapsed period of time.
  // CLOCK_MONOTONIC is used because it's guaranteed to move forward
  // at a constant rate, unaffected by system time changes or user adjustments.
  clock_gettime(CLOCK_MONOTONIC, &ts_start);
  sleep(1); // Wait for ~1 second to provide a long enough sample period.
  clock_gettime(CLOCK_MONOTONIC, &ts_end);
  // Read the final TSC value immediately after the system clock measurement.
  unsigned long long t1 = rdtsc_start();

  // Calculate the total elapsed time in seconds using the system clock.
  // This gives us an accurate and reliable time base.
  double elapsed_sec = (ts_end.tv_sec - ts_start.tv_sec) +
                       (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

  // Calculate the frequency by dividing the total cycles counted by
  // the total elapsed time. The result is cycles per second (Hertz).
  // This value is then used to convert cycle counts in the main program.
  return (double)(t1 - t0) / elapsed_sec;
}

// ----------------------------------------------------------------------------
// Main Program: Measures Syscall Time
// ----------------------------------------------------------------------------
int main(void) {
  printf("Starting RDTSC-based syscall measurement...\n");

  // --- PART 1: CALIBRATE CYCLES PER SECOND ---
  // This step is crucial for accurate timing. We can't use RDTSC
  // for absolute time without knowing the CPU's current frequency.
  printf("--------------------------------------------------\n");
  printf("1. Calibrate cycles per second\n");
  double cycles_per_sec = measure_tsc_freq();
  printf("Estimated CPU frequency: %.2f GHz\n", cycles_per_sec / 1e9);

  // --- PART 2: MEASURE SYSCALL CYCLES ---
  // Now that we have a frequency, we can use RDTSC to measure the
  // overhead of our target system call: a 0-byte read() from stdin.
  printf("--------------------------------------------------\n");
  printf("2. Measuring 0-byte read() syscall cycles...\n");
  printf("Running %d calls to get an average cycles.\n", NLOOPS);

  // Get the start TSC value just before the syscall loop begins.
  unsigned long long start_cycles = rdtsc_start();
  // Call the assembly function that performs all the syscalls in a loop.
  repeat_read_syscall(NLOOPS);
  // Get the end TSC value immediately after the loop has finished.
  unsigned long long end_cycles = rdtsc_start();

  // Calculate the total number of cycles elapsed for all syscalls.
  unsigned long long elapsed_syscall_cycles = end_cycles - start_cycles;
  // Calculate the average number of cycles per syscall.
  // This is our primary, raw measurement.
  double average_cycles = (double)elapsed_syscall_cycles / NLOOPS;
  // Convert the cycle count into a time measurement using the
  // calibrated frequency from Part 1.
  double cycles_per_us = cycles_per_sec / 1e6;
  double average_us = average_cycles / cycles_per_us;

  // Display the results.
  printf("Total elapsed cycles for %d syscalls: %llu cycles\n", NLOOPS,
         elapsed_syscall_cycles);
  printf("Average cycles per syscall: %.3f cycles\n", average_cycles);
  printf("--------------------------------------------------\n");
  printf("Average time per syscall: %.3f microseconds (μs)\n", average_us);
  printf("--------------------------------------------------\n");

  return EXIT_SUCCESS;
}

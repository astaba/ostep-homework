/* ostep-homework/ch06-ltd-direct-execution/syslaps-v1.c */
// Created on: Fri Sep 19 00:19:14 +01 2025
// Homework 6.1: Measure the cost of a syscall.

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define NLOOPS 1000000

// Return time difference in microseconds
long long get_time_diff(struct timeval *end, struct timeval *start) {
  return ((long long)end->tv_sec - start->tv_sec) * 1000000LL +
         ((long long)end->tv_usec - start->tv_usec);
}

int main(int argc, char *argv[argc + 1]) {
  printf("Starting system timer and syscall measurement...\n");
  // --- PART 1: MEASURE GETTIMEOFDAY() PRECISION ---
  printf("--------------------------------------------------\n");
  printf("1. Measuring gettimeofday() precision...\n");
  printf("Running %d back-to-back calls to find timer resolution.\n", NLOOPS);
  struct timeval start_time, end_time;
  long long elapsed_us = 0;
  long long min_nonzero_diff = -1;
  for (size_t i = 0; i < NLOOPS; i++) {
    // Run two back-to-back calls...
    gettimeofday(&start_time, NULL);
    gettimeofday(&end_time, NULL);
    // and measure the time between them.
    elapsed_us = get_time_diff(&end_time, &start_time);
    if (elapsed_us > 0) { // update the minimal time difference
      if (min_nonzero_diff == -1 || elapsed_us < min_nonzero_diff) {
        min_nonzero_diff = elapsed_us;
      }
    }
  }
  // Print the results of the precision test.
  if (min_nonzero_diff != -1) {
    printf("Minimum non-zero time difference: %lld microseconds (μs)\n",
           min_nonzero_diff);
    printf("This represents the approximate resolution of the system timer.\n");
  } else {
    printf("Could not measure a non-zero time difference.\n");
  }

  // --- PART 2: MEASURE EMPTY LOOP OVERHEAD ---
  printf("--------------------------------------------------\n");
  printf("2. Measuring overhead of %d empty loops...\n", NLOOPS);

  gettimeofday(&start_time, NULL);
  for (size_t i = 0; i < NLOOPS; i++) {
    // Loop intentionally empty to measure its own overhead.
  }
  gettimeofday(&end_time, NULL);

  long long empty_loop_time = get_time_diff(&end_time, &start_time);
  printf("Total elapsed time for empty loop: %lld microseconds (μs)\n",
         empty_loop_time);

  // --- PART 3: MEASURE SYSCALL TIME (WITH SUBTRACTION) ---
  printf("--------------------------------------------------\n");
  printf("3. Measuring 0-byte read() syscall time...\n");
  printf("Running %d calls to get an average time.\n", NLOOPS);

  /* char buffer[1]; */
  gettimeofday(&start_time, NULL);
  for (size_t i = 0; i < NLOOPS; i++) {
    // The read() call is a syscall.
    // We use file descriptor 0 (stdin) and a size of 0 to ensure it returns
    // immediately with no data transfer, measuring only the syscall overhead.
    ssize_t nread = read(STDIN_FILENO, NULL, 0);
    // Check for potential errors, though unlikely for a 0-byte read.
    if (nread == -1 || errno != 0) {
      perror("read() failed");
      exit(EXIT_FAILURE);
    }
  }
  gettimeofday(&end_time, NULL);

  long long elapsed_loop_time = get_time_diff(&end_time, &start_time);
  long long elapsed_syscall_time = elapsed_loop_time - empty_loop_time;

  double syscall_average = (double)elapsed_syscall_time / NLOOPS;

  printf("Total elapsed time for %d syscalls (with loop overhead subtracted): "
         "%lld microseconds (μs)\n",
         NLOOPS, elapsed_syscall_time);
  printf("--------------------------------------------------\n");
  printf("Average time per syscall: %.3f microseconds (μs)\n", syscall_average);
  printf("--------------------------------------------------\n");

  return EXIT_SUCCESS;
}

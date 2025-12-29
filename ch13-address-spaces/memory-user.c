/* ostep-homework/ch13-address-spaces/memory-user.c */
// Created on: Mon Oct 20 23:15:21 +01 2025
// Homework 13.3: Create a little program that uses a certain amount of memory.
// This program should take one command-line argument:
// the number of megabytes of memory it will use. When run, it should allocate
// an array, and constantly stream through the array, touching each entry.
// The program should do this indefinitely, or, perhaps, for a certain amount
// of time also specified at the command line.
// NOTE:
// Run in terminal and monitor in another terminal with either commands:
// $ watch -n 1 free -m
// $ free -m -s 1

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Define standard page size for efficient "touching" of memory page.
#define PAGE_SIZE 4096

int main(int argc, char *argv[argc + 1]) {

  // Validate required argument: the number of megabytes (MB) to allocate.
  // Display usage message.
  if (argc < 2) {
    fprintf(stderr,
            "Usage: %s <megabytes_to_use> [time_seconds]\n"
            "  <megabytes_to_use>: The amount of memory to allocate (in MB).\n"
            "  [time_seconds]: Optional. The duration to run (in "
            "seconds). Runs indefinitely if omitted.\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }

  // 1. Parse required arguments and convert it from MB to MiB
  long mb_to_use = strtol(argv[1], NULL, 10);
  if (mb_to_use <= 0) {
    fprintf(stderr, "Error: Megabytes must be a positive number.\n");
    exit(EXIT_FAILURE);
  }
  size_t bytes_to_allocate = (size_t)mb_to_use * 1024 * 1024;

  // Parse optional argument: how long to run in seconds.
  long run_time_seconds = -1; // Default: run indefinitely.
  if (argc >= 3) {
    run_time_seconds = strtol(argv[2], NULL, 10);
    if (run_time_seconds <= 0) {
      fprintf(stderr, "Warning: invalid time. Running indefinitely.\n");
      run_time_seconds = -1;
    }
  }

  // Display process ID.
  printf("pid: %d\n", getpid());

  printf("--- Memory Allocator Utility ---\n");
  printf("Attempting to allocate %ld MB (%zu bytes).\n", mb_to_use,
         bytes_to_allocate);
  if (run_time_seconds > 0) {
    printf("Running for %ld seconds.\n", run_time_seconds);
  } else {
    printf("Running indefinitely until killed (Ctrl+C)\n");
  }

  // 2. Allocate memory.
  // The memory is allocated, but physical pages aren't commited yet.
  char *memory_array = (char *)malloc(bytes_to_allocate);
  if (!memory_array) {
    perror("malloc() failed");
    fprintf(stderr, "Could not allocate %ld MB.\n", mb_to_use);
    exit(EXIT_FAILURE);
  }
  printf("Memory successfully allocated.\n");

  /* 3. Mark all pages as "dirty" initially.
   * We touch every page (PAGE_SIZE = 4096 bytes) to force the OS to commit
   * physical pages and count this memory as dirty. The purpose is to
   * pre-emptively incur in the time-cost of initial TLB miss, subsequent page
   * fault and subsequent demand paging. */
  for (size_t i = 0; i < bytes_to_allocate; i += PAGE_SIZE) {
    memory_array[i] = 0xAA; // Write a distinct value.
  }
  printf("Initial memory pages have been touch (dirty).\n");

  // 4. Continuous streaming loop.
  time_t start_time = time(NULL);
  time_t end_time = start_time + run_time_seconds;
  size_t access_counter = 0;

  printf("Starting continuous memory stream...\n");

  // The loop continues indefinitely or until the run_time is reached.
  while (run_time_seconds < 0 || time(NULL) < end_time) {
    // Loop through the array touching one byte per page to ensure high memory
    // usage.
    for (size_t i = 0; i < bytes_to_allocate; i += PAGE_SIZE) {
      // Volatile cast to prevent the compiler from optimizing away the write
      // access, garanteeing that the memory access happens and the pages
      // remain "dirty".
      *(volatile char *)&memory_array[i] = (char)access_counter;
    }

    access_counter++;
  }

  printf("Time limit reached. Freeing memory and exit.\n");
  free(memory_array);

  return EXIT_SUCCESS;
}

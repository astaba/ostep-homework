#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Simple routine to return absolute time (in seconds).
double Time_GetSeconds() {
  struct timeval t;
  int rc = gettimeofday(&t, NULL);
  assert(rc == 0);
  return (double)((double)t.tv_sec + (double)t.tv_usec / 1e6);
}

/*
  FIXED VERSION
  1. Increments total_loops immediately when a full array pass is done.
  2. Calculates bandwidth based on ALL work done in the time window,
  not just a snapshot of the single last loop.
 */
int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: spin <memory (MB)>\n");
    exit(1);
  }

  long long int size_mib = atoll(argv[1]);
  long long int size_b = size_mib * 1024 * 1024;
  long long int num_ints = size_b / sizeof(int);
  printf("Allocating %lld bytes (%.2f MB)\n", size_b, size_mib * 1.0);

  int *x = malloc(size_b);
  if (x == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  printf("\tnumber of integers in array: %lld\n", num_ints);
  // Touch all memory first to trigger demand paging before timing starts
  memset(x, 0, size_b);

  long long int i = 0;
  int total_loops = 0;
  int loops_in_window = 0;
  double window_start_time = Time_GetSeconds();

  while (1) {
    x[i++] += 1; // Perform the memory write
    // Check if we finished one full pass of the array
    if (i == num_ints) {
      total_loops++;
      loops_in_window++;
      i = 0; // Reset array index
      double current_time = Time_GetSeconds();
      double elapsed_window = current_time - window_start_time;
      /**
       * WINDOW DEFINITION:
       * To prevent the console from being flooded and to provide more stable
       * performance metrics, we use an arbitrary time threshold (0.2 seconds).
       * A "window" refers to the accumulation of all full array passes (loops)
       * that occurred within this specific time slice. The bandwidth reported
       * is the average performance across this entire window.
       */
      if (elapsed_window >= 0.2) {
        double window_total_mib = (double)loops_in_window * size_mib;
        // Calculate bandwidth: (window work) / ( window time)
        double bandwidth = window_total_mib / elapsed_window;
        printf("Loops: %-8d | Window: %4d loops at %6.2f ms/lp | Bandwidth: "
               "%8.2f MB/s\n",
               total_loops, loops_in_window,
               elapsed_window * 1000.0 / loops_in_window, bandwidth);

        // Reset trackers for the next measurement window
        loops_in_window = 0;
        window_start_time = current_time;
      }
    }
  }

  return 0;
}

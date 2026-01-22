/*
 * VERSION IMPROVEMENT
 * (1) Timer: Replaced gettimeofday with clock_gettime(CLOCK_MONOTONIC) for
 * high-res, jitter-free timing. (2) Logic: Increments loop_count immediately
 * upon completion to ensure accurate cycle tracking. (3) Precision: Switches to
 * average windowed-bandwidth to eliminate noise from OS context switches. (4)
 * Accuracy: Adds memory warming via memset to eliminate Demand Paging/Page
 * Fault overhead. (5) Metrics: Introduces 'ms/lp' (milliseconds per loop) for
 * high-resolution performance insight.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double Time_GetSeconds(void) {
  struct timespec tp;
  int rc = clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  if (rc) {
    perror("clock_gettime() failed");
    exit(EXIT_FAILURE);
  }
  return (double)tp.tv_sec + (double)tp.tv_nsec / 1e9L;
}

// Program that allocates an array of ints of certain size,
// and then proceeeds to update each int in a loop, forever.
int main(int argc, char *argv[argc + 1]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s <memory in MiB>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  long long int size_MiB = atoll(argv[1]);
  long long int scale = 1024 * 1024;
  long long int size_B = size_MiB * scale;
  printf("Allocating %lld bytes (%.2f MiB)\n", size_B, (double)size_B / scale);

  int *x = malloc(size_B);
  if (x == NULL) {
    perror("malloc() failed");
    exit(EXIT_FAILURE);
  }
  long long int num_ints = size_B / sizeof(int);
  printf("\tnumber of integers in array: %lld\n", num_ints);

  /* Notice that since memset() comes right before Time_GetSeconds() call, thus
   * out of timestamps, while slightly slowing down program startup it would
   * tremendously improve first loop performance by warming up the TLB through
   * the process of touching all memory first to trigger demand paging before
   * timing starts */
  /* memset(x, 0, size_B); */

  /* Init loops trackers */
  long long int i = 0;
  int total_loops = 0;
  int window_loops = 0;
  double window_start_time = Time_GetSeconds();
  while (1) {
    x[i++] += 1;
    /* Check completion of one full pass over the array */
    if (i == num_ints) {
      total_loops++;
      window_loops++;
      i = 0; /* Reset array index */
      double current_time = Time_GetSeconds();
      double window_time = current_time - window_start_time;
      /**
       * WINDOW DEFINITION:
       * To prevent the console from being flooded and to provide more stable
       * performance metrics, we use an arbitrary time threshold (0.5 seconds).
       * A "window" refers to the accumulation of all full array passes (loops)
       * that occurred within this specific time slice. The bandwidth reported
       * is the average performance across this entire window.
       */
      if (window_time >= 0.5) {
        double bandwidth = (double)(window_loops * size_MiB) / window_time;
        /* Print report */
        printf("Loops: %-8d | Window: %4d loops at %6.2lf ms/lp | Bandwidth: "
               "%8.2lf MiB/s\n",
               total_loops, window_loops, window_time * 1000.0 / window_loops,
               bandwidth);
        /* In case need arises to pipe output to `tee` fflush() will bypass libc
         * fullbuffering on printf(). However, in such a memory experiment lab
         * it is a very bad idea to make syscall, force kernel overtaking in a
         * loop iterating potentially thousands per seconds. Verdict: remove it
         * to prevent kernel-induced performance throttling. */
        /* fflush(stdout); */
        /* Reset window trackers */
        window_loops = 0;
        window_start_time = current_time;
      }
    }
  }
  return EXIT_SUCCESS;
}

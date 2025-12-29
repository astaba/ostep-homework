/* ostep-homework/ch19-vm-tlb/tlb.c */
/* Created on: Fri Dec  5 08:45:16 +01 2025 */

/* Q1: The precision of the choosen timer directly influence the duration
 * (in this case the number of trials) of the operation must have to be timed
 * precisely.
 * Q2: Input parameter: number of page to touch; number of trials.
 * Q3: Write a script to run tlb.c passing incremental number of pages from 1 to
 * a few thousand, incrementing by a factor of 2.
 * Q4: Graph the results.
 * Q5: Make sure the compiler does not optimize  the loop away.
 * Q6: Make sure code runs on only one CPU.
 * Q7: Make sure the timing measurement start with a hot TLB. In other word
 * trigger in anticipation: TLB miss, page fault and Kernel's demand-zero
 * resulting in actual DRAM allocation and the Page Table Entries to be
 * validated for a hot TLB.
 */

#define _GNU_SOURCE /* feature test macro for cpu set macros from sched.h */

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static inline int numer(const char *str, long long *const n) {
  char *end = NULL;
  errno = 0;
  *n = strtoll(str, &end, 10);
  if (errno != 0) {
    perror("strtol() failed");
    return 1;
  }
  if (str == end) {
    fprintf(stderr, "strtol() error: No digits were found in '%s'.\n", str);
    return 1;
  }
  if (*n > 0 && *end != 0) {
    fprintf(stderr, "strtol() error: Extra character in '%s'.\n", str);
    return 1;
  }
  if (*n <= 0) {
    fprintf(stderr, "strtol() error: Requires a positive integer '%s'.\n", str);
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[argc + 1]) {
  /* Check for required arguments */
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <pages_number> <trials_number>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Pin process to one CPU */
  cpu_set_t cpu_set;
  int cpu_no = 0;
  CPU_ZERO(&cpu_set);
  CPU_SET(cpu_no, &cpu_set);
  if (sched_setaffinity(getpid(), sizeof(cpu_set), &cpu_set) == -1) {
    perror("sched_setaffinity() failed");
    exit(EXIT_FAILURE);
  }
  assert(CPU_ISSET(cpu_no, &cpu_set));

  /* Parse both arguments */
  long long page_no, trial_no;
  if (numer(argv[1], &page_no) || numer(argv[2], &trial_no))
    exit(EXIT_FAILURE);

  /* Get system page size */
  int page_size;
  if ((page_size = (int)sysconf(_SC_PAGESIZE)) == -1) {
    perror("sysconf() failed");
    exit(EXIT_FAILURE);
  }

  /* Allocate dynamic memory for an array */
  long long total_size = page_no * page_size;

  volatile char *volatile array = malloc(total_size);
  if (array == NULL) {
    perror("malloc() failed");
    exit(EXIT_FAILURE);
  }

  /* Warm up TLB by forcing DRAM allocation and PTEs population */
  for (size_t i = 0; i < page_no; i++) {
    array[i * page_size] = (char)i;
  }

  /* Set up time measurement */
  struct timespec ts_start, ts_end;
  volatile long long dummy_sum = 0;

  if (clock_gettime(CLOCK_MONOTONIC, &ts_start) == -1) {
    perror("clock_gettime() failed");
    free((void *)array);
    exit(EXIT_FAILURE);
  }

  /* Run the DRAM updates requiring TLB accesses */
  for (long long i = 0; i < trial_no; i++) {
    for (long long j = 0; j < page_no; j++) {
      dummy_sum += array[j * page_size];
    }
  }

  if (clock_gettime(CLOCK_MONOTONIC, &ts_end) == -1) {
    perror("clock_gettime() failed");
    free((void *)array);
    exit(EXIT_FAILURE);
  }

  /* Compute pages access time */
  double start_ns = (double)ts_start.tv_sec * 1e9 + (double)ts_start.tv_nsec;
  double end_ns = (double)ts_end.tv_sec * 1e9 + (double)ts_end.tv_nsec;
  double diff_ns = end_ns - start_ns;

  /* WARN: The target time is not the array streaming but the time to get each
   * and every single page's PTE: whether it met a TLB hit or miss. */
  double total_access = (double)trial_no * page_no;
  double average_time = diff_ns / total_access;

  printf("%5lld %8lld %6.2lf\n", page_no, total_size, average_time);

  /* Cleanup */
  free((void *)array);
  return EXIT_SUCCESS;
}

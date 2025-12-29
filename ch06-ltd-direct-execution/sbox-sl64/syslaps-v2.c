/* ostep-homework/ch06-ltd-direct-execution/sbox-sl64/syslaps-v2.c */
/* Created on: Tue Dec 16 20:03:39 +01 2025 */
/* Homework 6.1: Measure the cost of a syscall.
 * Build:
 *  nasm -f elf64 timestamp64.asm -o timestamp64.o
 *  gcc syslaps-v2.c timestamp64.o -o syslaps-v2.out
 */
/* Don't base performance measures on constants like CLOCKS_PER_SC because
 * modern CPUs have dynamic speed. Use the RDTSC register and clock_gettime() to
 * compute the current CPU frequency. Then use again RDTSC before and after the
 * target performance to have its extremely precise number of cycles. At the end
 * use that current frequency to have the duration. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NLOOPS 1000000

extern unsigned long long rdtsc_call(void);
extern void repeat_read_syscall(unsigned int loopno);

double get_cpu_freq(void) {
  unsigned long long cycle0, cycle1;
  struct timespec ts_start, ts_end;

  cycle0 = rdtsc_call();
  clock_gettime(CLOCK_MONOTONIC, &ts_start);
  sleep(1);
  clock_gettime(CLOCK_MONOTONIC, &ts_end);
  cycle1 = rdtsc_call();

  long long laps = (ts_end.tv_sec - ts_start.tv_sec) +
                   (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

  return (cycle1 - cycle0) / (double)laps;
}

int main(int argc, char *argv[argc + 1]) {
  printf("=== Compute CPU Frequency ===\n");
  double cpufreq_per_s = get_cpu_freq();
  printf("CPU frequency: %.2lf GigaHerz\n", cpufreq_per_s / 1e9);

  unsigned long long cycle0, cycle1, avg_cycles;
  cycle0 = rdtsc_call();
  repeat_read_syscall(NLOOPS);
  cycle1 = rdtsc_call();

  avg_cycles = (cycle1 - cycle0) / NLOOPS;
  /* Compute syscall time in microseconds */
  double sys_time = (avg_cycles / cpufreq_per_s) * 1e6;

  printf("Average duration of sys_read: %.2lf μs\n", sys_time);

  return EXIT_SUCCESS;
}

/* ostep-homework/ch06-ltd-direct-execution/cswlaps-v3.c */
// Created on: Sat Sep 20 02:20:10 +01 2025
// Homework 6.2: Measure the cost of a context switch.
// You can see number and status of context switch with:
// sudo time -v <executable>
// INFO: New Refinements to Benchmark
// 1. Warm-up iterations: Do a small warm-up loop (e.g., 10,000 exchanges)
//    before measuring, to “prime” the kernel path and caches.
// 2. Subtract syscall baseline: Measure back-to-back write+read in a single
//    process (no fork) on the same pipe, to estimate pure syscall cost.
//    Subtract this from your ping-pong measurement → leaves you closer to just
//    context switch cost.
// 3. Try eventfd: Another neat primitive that can force context switches, with
//    even lighter weight than socketpair.

#define _GNU_SOURCE /* See feature_test_macros(7) */

#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Number of context switches to perform. A large number
// is used to get a stable average by smoothing out noise.
#define NLOOPS 1000000
#define WARMUP_LOOPS 10000
// We will pin both processes to this CPU to force context switches.
// We use a single logical CPU to prevent parallel execution.
#define CPU_TO_PIN 0

#define CHECKED_EVENTFDS_IO(RET, SCALL, FD, VAL)                               \
  RET = SCALL(FD, &VAL, sizeof(VAL));                                          \
  if (RET == -1) {                                                             \
    perror(#SCALL "() failed");                                                \
    exit(EXIT_FAILURE);                                                        \
  }

#define CHECKED_PIPEFDS_IO(RET, SCALL, FD, DATA)                               \
  RET = SCALL(FD, DATA, 1);                                                    \
  if (RET == -1) {                                                             \
    perror(#SCALL "() failed");                                                \
    exit(EXIT_FAILURE);                                                        \
  }

// Helper function to set the CPU affinity. It pins the current
// process to the specified logical CPU.
void set_cpu_affinity(int cpu) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);
  if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
    perror("sched_setaffinity failed");
    exit(EXIT_FAILURE);
  }
}

// Helper function to get the current time in nanoseconds using
// CLOCK_MONOTONIC_RAW. This is used because it's a high-resolution, monotonic
// timer.
long long get_time_ns() {
  struct timespec ts;
  // CLOCK_MONOTONIC_RAW is used because it is not affected by NTP or other
  // system time adjustments.
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return (long long)ts.tv_sec * 1e9 + ts.tv_nsec;
}

// Measures the baseline cost of IPC by performing a back-to-back write and read
// without a context switch.
long long measure_ipc_baseline(int read_fd, int write_fd, int is_pipe) {
  char data[1] = {0};
  long long start_time, end_time;
  ssize_t ret;

  if (is_pipe) {
    start_time = get_time_ns();
    for (int i = 0; i < NLOOPS; i++) {
      CHECKED_PIPEFDS_IO(ret, write, write_fd, data)
      CHECKED_PIPEFDS_IO(ret, read, read_fd, data)
    }
    end_time = get_time_ns();
  } else { // eventfd
    uint64_t val = 1;
    start_time = get_time_ns();
    for (int i = 0; i < NLOOPS; i++) {
      CHECKED_EVENTFDS_IO(ret, write, read_fd, val)
      CHECKED_EVENTFDS_IO(ret, read, read_fd, val)
    }
    end_time = get_time_ns();
  }

  return (end_time - start_time);
}

// Worker process loop.
void run_child(int read_fd, int write_fd, int is_pipe) {
  set_cpu_affinity(CPU_TO_PIN);
  char buf[1] = {0};
  uint64_t val = 0;
  ssize_t ret;

  for (int i = 0; i < NLOOPS + WARMUP_LOOPS; i++) {
    if (is_pipe) {
      CHECKED_PIPEFDS_IO(ret, read, read_fd, buf)
      CHECKED_PIPEFDS_IO(ret, write, write_fd, buf)
    } else { // eventfd
      CHECKED_EVENTFDS_IO(ret, read, read_fd, val)
      CHECKED_EVENTFDS_IO(ret, write, write_fd, val)
    }
  }
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  if (sysconf(_SC_NPROCESSORS_ONLN) < 2) {
    fprintf(stderr, "This program requires at least 2 logical CPUs.\n");
    exit(EXIT_FAILURE);
  }

  // --- Determine IPC method from arguments ---
  int is_pipe = 1; // Default to pipes
  if (argc > 1) {
    if (strcmp(argv[1], "eventfd") == 0) {
      is_pipe = 0;
    } else if (strcmp(argv[1], "pipe") != 0) {
      fprintf(stderr, "Usage: %s [pipe|eventfd]\n", argv[0]);
      return EXIT_FAILURE;
    }
  }

  // --- Setup IPC Mechanism ---
  // The previous version had a bug using a 2-element array for 2 pipes.
  // This corrected version uses a 4-element array to hold all fds safely.
  int fds[4];
  if (is_pipe) {
    // Create pipe for parent -> child communication.
    // fds[0] is the read end, fds[1] is the write end.
    if (pipe(fds) == -1) {
      perror("pipe creation failed");
      return EXIT_FAILURE;
    }
    // Create a second pipe for child -> parent communication.
    // fds[2] is the read end, fds[3] is the write end.
    if (pipe(fds + 2) == -1) {
      perror("pipe creation failed");
      return EXIT_FAILURE;
    }
  } else {
    // We create two separate eventfds to allow two-way communication.
    // fds[0] for parent -> child, fds[1] for child -> parent.
    if ((fds[0] = eventfd(0, EFD_CLOEXEC)) == -1 ||
        (fds[1] = eventfd(0, EFD_CLOEXEC)) == -1) {
      perror("eventfd creation failed");
      return EXIT_FAILURE;
    }
  }

  // --- Measure IPC baseline cost ---
  printf("Measuring IPC baseline cost...\n");
  // For the baseline measurement, we use the first pipe's descriptors.
  long long baseline_cost = measure_ipc_baseline(fds[0], fds[1], is_pipe);
  printf("IPC baseline (write+read) cost: %.2f ns\n",
         (double)baseline_cost / NLOOPS);

  // --- Fork Process ---
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork() failed");
    return EXIT_FAILURE;
  }

  if (pid == 0) {
    // Child Process
    if (is_pipe) {
      // The child only needs the read end of pipe1 and the write end of pipe2.
      // Close the ends it doesn't need.
      close(fds[1]); // Close write end of pipe1
      close(fds[2]); // Close read end of pipe2

      // The child uses fds[0] for reading and fds[3] for writing.
      run_child(fds[0], fds[3], is_pipe);
    } else {
      // NOTE: For eventfd, the child needs both fds to communicate.
      // There are no fds to close. The parent and child simply use
      // the same fds in reverse read/write directions.
      // The child reads from fds[0] and writes to fds[1].
      run_child(fds[0], fds[1], is_pipe);
    }

  } else {
    // Parent Process
    if (is_pipe) {
      // The parent only needs the write end of pipe1 and the read end of pipe2.
      // Close the ends it doesn't need.
      close(fds[0]); // Close read end of pipe1
      close(fds[3]); // Close write end of pipe2
    }

    // Pin both parent and child to CPU_TO_PIN.
    set_cpu_affinity(CPU_TO_PIN);

    char buf[1] = {0};
    uint64_t val = 1;
    long long start, end;
    ssize_t ret;

    // --- Warm-up loop ---
    printf("Running warm-up loop (%d iterations)...\n", WARMUP_LOOPS);
    for (int i = 0; i < WARMUP_LOOPS; ++i) {
      if (is_pipe) {
        // Parent writes to pipe1 (fds[1]), and reads from pipe2 (fds[2]).
        CHECKED_PIPEFDS_IO(ret, write, fds[1], buf)
        CHECKED_PIPEFDS_IO(ret, read, fds[2], buf)
      } else { // eventfd
        // Parent writes to fds[0] and reads from fds[1].
        CHECKED_EVENTFDS_IO(ret, write, fds[0], val)
        CHECKED_EVENTFDS_IO(ret, read, fds[1], val)
      }
    }

    // --- Main Measurement Loop ---
    // This is the BENCHMARK LOOP
    printf("Starting main measurement loop (%d iterations)...\n", NLOOPS);
    start = get_time_ns();
    for (int i = 0; i < NLOOPS; i++) {
      if (is_pipe) {
        // Parent writes to pipe1 (fds[1]), and reads from pipe2 (fds[2]).
        CHECKED_PIPEFDS_IO(ret, write, fds[1], buf)
        CHECKED_PIPEFDS_IO(ret, read, fds[2], buf)
      } else { // eventfd
        // Parent writes to fds[0] and reads from fds[1].
        CHECKED_EVENTFDS_IO(ret, write, fds[0], val)
        CHECKED_EVENTFDS_IO(ret, read, fds[1], val)
      }
    }
    end = get_time_ns();

    // Wait for the child to finish.
    wait(NULL);

    // --- Calculate and Print Results ---
    long long total_elapsed_ns = end - start;
    // The total number of context-switching in the Benchmark loop
    long long nswitches = NLOOPS * 2;
    // Time cost of single context switch comprised of all IPC overheads.
    double bulk_switch_ns = (double)total_elapsed_ns / nswitches;
    // Pure time cost of a single context-switching deprived as much as possible
    // of IPC overheads. Subtract the baseline IPC cost to get a cleaner context
    // switch cost.
    double pure_switch_ns =
        (double)(total_elapsed_ns - 2 * baseline_cost) / nswitches;

    printf("--------------------------------------------------\n");
    printf("Context Switch Cost Measurement\n");
    printf("Mechanism: %s\n", is_pipe ? "Pipes" : "Eventfd");
    printf("--------------------------------------------------\n");
    printf("Total elapsed time: %.3f ms\n", total_elapsed_ns / 1e6);
    printf("Average cost per total switch (incl. IPC): %.3f ns\n",
           bulk_switch_ns);
    printf("Average PURE context switch cost (est.): %.3f ns\n",
           pure_switch_ns);
    printf("--------------------------------------------------\n");

    if (is_pipe) {
      close(fds[1]);
      close(fds[2]);
    } else {
      close(fds[0]);
      close(fds[1]);
    }
  }

  return EXIT_SUCCESS;
}

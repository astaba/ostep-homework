/* ostep-homework/ch06-ltd-direct-execution/cswlaps-v2.c */
// Created on: Sat Sep 20 00:29:15 +01 2025
// Homework 6.2: Measure the cost of a context switch.
// You can see number and status of context switch with:
// sudo time -v <executable>

#define _GNU_SOURCE /* See feature_test_macros(7) */

#include <assert.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define CPU_TO_PIN 0
#define NLOOPS 1000000

void set_cpu_affinity(int cpu_to_pin) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu_to_pin, &mask);
  sched_setaffinity(0, sizeof(cpu_set_t), &mask);
}

long long get_nano_laps(void) {
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  return (long long)(tp.tv_sec * 1e9 + tp.tv_nsec);
}

int main(int argc, char *argv[argc + 1]) {
  // --- Verify CPU Affinity Support ---
  // Check if the current system has multiple CPUs and supports affinity.
  if (sysconf(_SC_NPROCESSORS_ONLN) < 2) {
    fprintf(stderr, "This program requires at least 2 logical CPUs.\n");
    exit(EXIT_FAILURE);
  } else {
    printf("SYSCONFIG INFO:\n"
           "%d processors available out of %d processors configured.\n",
           get_nprocs(), get_nprocs_conf());
  }

  // --- Setup Pipes ---
  // Create two pipes for communication between the parent and child processes.
  // The first pipe is for the parent to write and the child to read.
  // The second pipe is for the child to write and the parent to read.
  int oyako_pipe[2];
  int kooya_pipe[2];
  if (pipe(oyako_pipe) == -1 || pipe(kooya_pipe) == -1) {
    perror("pipe() failed");
    exit(EXIT_FAILURE);
  }

  // --- Fork Process ---
  // Create the child process. The fork() call returns 0 to the child and
  // the child's PID to the parent.
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork() failed");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) { // Child process
    // 1. Set cpu affinity
    set_cpu_affinity(CPU_TO_PIN);
    // 2. Close unused pipes file descriptors
    close(oyako_pipe[1]);
    close(kooya_pipe[0]);
    // 3. Run the bidirectional communication loop starting by listening
    char buf[1] = {0};
    for (size_t i = 0; i < NLOOPS; i++) {
      if (read(oyako_pipe[0], buf, 1) == -1) {
        perror("read() from parent failed");
        exit(EXIT_FAILURE);
      }
      if (write(kooya_pipe[1], buf, 1) == -1) {
        perror("write() to parent failed");
        exit(EXIT_FAILURE);
      }
    }
    // 4. Close remaining pipes file descriptors
    close(oyako_pipe[0]);
    close(kooya_pipe[1]);

    exit(EXIT_SUCCESS);

  } else { // Parent process
    printf("--------------------------------------------------\n");
    printf("Context Switch Cost Measurement\n");
    printf("Through PIPE communication\n");
    printf("--------------------------------------------------\n");
    // 1. Set cpu affinity
    set_cpu_affinity(CPU_TO_PIN);
    // 2. Close unused pipes file descriptors
    close(oyako_pipe[0]);
    close(kooya_pipe[1]);
    // 3. Commit loop entry nanotime
    long long start, end;
    start = get_nano_laps();
    // 4. Run the bidirectional communication loop starting by writing
    char buf[1] = {0};
    for (size_t i = 0; i < NLOOPS; i++) {
      if (write(oyako_pipe[1], buf, 1) == -1) {
        perror("write() to child failed");
        exit(EXIT_FAILURE);
      }
      if (read(kooya_pipe[0], buf, 1) == -1) {
        perror("read() from child failed");
        exit(EXIT_FAILURE);
      }
    }
    // 5. Commit loop exit nanotime
    end = get_nano_laps();
    // 6. Wait child process to return.
    pid_t wpid = wait(NULL);
    assert(pid == wpid);
    // 7. Compute elapsed time. Compute per_switch_cost.
    long long elapsed = end - start;
    double per_switch_cost = (double)elapsed / (NLOOPS * 2);
    // 8. Display results.
    printf("Number of switches: %lld\n", (long long)(NLOOPS * 2));
    printf("Total elapsed time: %.3f ms\n", elapsed / 1e6);
    printf("Average cost per switch: %.3f nanoseconds (ns)\n", per_switch_cost);
    printf("--------------------------------------------------\n");
    // 9. Close remaining pipes file descriptors
    close(oyako_pipe[1]);
    close(kooya_pipe[0]);

    return EXIT_SUCCESS;
  }
}

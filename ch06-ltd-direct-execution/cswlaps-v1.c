/* ostep-homework/ch06-ltd-direct-execution/cswlaps-v1.c */
// Created on: Fri Sep 19 16:23:33 +01 2025
// Homework 6.2: Measure the cost of a context switch.
// You can see number and status of context switch with:
// sudo time -v <executable>

#define _GNU_SOURCE /* See feature_test_macros(7) */

#include <assert.h>
#include <bits/time.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define CPU_TO_PIN 0
#define NLOOPS 1000000

void pin_to_cpu(void) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(CPU_TO_PIN, &mask);
  sched_setaffinity(0, sizeof(cpu_set_t), &mask);
}

int main(int argc, char *argv[argc + 1]) {
  // 1. Make sure the system has more than 1 logical processors.
  if (sysconf(_SC_NPROCESSORS_ONLN) < 2) {
    fprintf(stderr, "This program requires at least 2 logical CPUs.\n");
    exit(EXIT_FAILURE);
  } else {
    printf("SYSCONFIG INFO:\n"
           "%d processors available out of %d processors configured.\n",
           get_nprocs(), get_nprocs_conf());
  }
  // 2. Create bidirectional and unnamed pair of connected sockets.
  // Declare file descriptors set
  int fds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
    perror("socketpair() failed");
    exit(EXIT_FAILURE);
  }
  // 3. Create child process to establish IPC with.
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork() failed");
    exit(EXIT_FAILURE);
  }
  // 4. Pin both parent and child to CPU_TO_PIN.
  pin_to_cpu();

  // 5. Child process carries on IPC by listening first.
  if (pid == 0) {
    // Close the unused end of the socket to prevent leak.
    close(fds[0]);

    char buf[1];
    while (1) {
      read(fds[1], buf, 1);
      write(fds[1], buf, 1);
    }

    close(fds[1]);
    return EXIT_SUCCESS;

  } else {
    // 6. Parent process establishes IPC
    // Close the unused end of the socket to prevent leak.
    close(fds[1]);

    printf("--------------------------------------------------\n");
    printf("Context Switch Cost Measurement\n");
    printf("Through SOCKET communication\n");
    printf("--------------------------------------------------\n");

    char buf[1] = {0};
    struct timespec start, end;

    // 7. Run the IPC loop incurring context-switching
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    for (size_t i = 0; i < NLOOPS; i++) {
      write(fds[0], buf, 1);
      read(fds[0], buf, 1);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    // WARNING: 8.
    // If you were to wait() for the child process to terminate before calling
    // close() the parent would stall indefinitely because, du to the socket API
    // fds specifications the child's read() call blocks while the parent's end
    // is open unbeknown to him that the parent has already terminated its loop
    // making the write() call.
    close(fds[0]);

    // Wait for the child process to exit as a good practice.
    pid_t wpid = wait(NULL);
    assert(pid == wpid);

    // 9. Perform relevant measures.
    long long elapsed_ns =
        (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

    double per_switch_cost = (double)elapsed_ns / (2.0 * NLOOPS);

    // 8. Display results.
    printf("Number of switches: %lld\n", (long long)(NLOOPS * 2));
    printf("Total elapsed time: %.3f seconds (s)\n", elapsed_ns / 1e9);
    printf("--------------------------------------------------\n");
    printf("Average cost per context-switching: %g microseconds (μs)\n",
           per_switch_cost / 1e3);
    printf("--------------------------------------------------\n");
    // 9. Close remaining pipes file descriptors
  }

  return EXIT_SUCCESS;
}

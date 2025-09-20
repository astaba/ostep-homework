/* ostep-homework/ch06-ltd-direct-execution/cswlaps-v1.c */
// Created on: Fri Sep 19 16:23:33 +01 2025
// Homework 6.2: Measure the cost of a context switch.
// You can see number and status of context switch with:
// sudo time -v <executable>

#define _GNU_SOURCE
#include <assert.h>
#include <bits/time.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
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
  if (sysconf(_SC_NPROCESSORS_ONLN) < 2) {
    fprintf(stderr, "This program requires at least 2 logical CPUs.\n");
    exit(EXIT_FAILURE);
  } else {
    printf("SYSCONFIG INFO:\n"
           "%d processors available out of %d processors configured.\n",
           get_nprocs(), get_nprocs_conf());
  }
  /* code */
  // Declare file descriptors set
  int fds[2];
  // Create unamed pair of connected sockets
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
    perror("socketpair() failed");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork() failed");
    exit(EXIT_FAILURE);
  }
  pin_to_cpu(); // Pin both parent and child to CPU_TO_PIN

  if (pid == 0) {
    char buf[1];
    while (1) {
      read(fds[1], buf, 1);
      write(fds[1], buf, 1);
    }
  } else {
    printf("--------------------------------------------------\n");
    printf("Context Switch Cost Measurement\n");
    printf("Through SOCKET communication\n");
    printf("--------------------------------------------------\n");

    char buf[1] = {0};
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    for (size_t i = 0; i < NLOOPS; i++) {
      write(fds[0], buf, 1);
      read(fds[0], buf, 1);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    long long elapsed_ns =
        (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

    double per_switch = (double)elapsed_ns / (2.0 * NLOOPS);

    printf("Average context switch time: %.2f ns\n", per_switch);
    // 8. Display results.
    printf("Number of switches: %lld\n", (long long)(NLOOPS * 2));
    printf("Total elapsed time: %.3f ms\n", elapsed_ns / 1e6);
    printf("Average cost per switch: %.3f nanoseconds (ns)\n", per_switch);
    printf("--------------------------------------------------\n");
    // 9. Close remaining pipes file descriptors
  }

  return EXIT_SUCCESS;
}

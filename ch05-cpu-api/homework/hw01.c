// Homework 5.01
// Write a program that calls fork(). Before calling fork(), have the main
// process access a variable (e.g., x) and set its value to something (e.g.,
// 100). What value is the variable in the child process? What happens to the
// variable when both the child and parent change the value of x?

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  int x = 100;
  printf("(pid:%d) Main process: x = %d\n", getpid(), x);

  int rc = fork();

  if (rc < 0) {
    fprintf(stderr, "fork() failed.\n");
    exit(EXIT_FAILURE);

  } else if (rc == 0) {
    printf("(pid:%d) Child process: x = %d\n", getpid(), x);
    printf("(pid:%d) Child process changing x value.\n", getpid());
    x = 99;

  } else {
    printf("(pid:%d) Main process spawned child:%d: x = %d\n", getpid(), rc, x);
    printf("(pid:%d) Main process changing x value.\n", getpid());
    x = 111;
  }

  printf("(pid:%d) End: x = %d\n", getpid(), x);
  return EXIT_SUCCESS;
}

// Homework 5.05
// Now write a program that uses wait() to wait for the child process to finish
// in the parent. What does wait() return? What happens if you use wait() in the
// child?

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {

  printf("(pid:%d) Hello, world!\n", getpid());

  int rc = fork();

  if (rc < 0) {
    perror("fork() failed");
    exit(EXIT_FAILURE);

  } else if (rc == 0) {
    // TEST: Try calling wait() in the child process
    int pid = getpid();
    int status = wait(NULL);
    if (status == -1)
      fprintf(stderr, "(pid:%d) wait() failed: %s\n", pid, strerror(errno));

    printf("(pid:%d) Hi, I am the child process\n", pid);
    for (int i = 0; i < 3; i++) {
      printf("(pid:%d) Let's sing again...\n", pid);
      usleep(200000);
    }
  } else {
    int sing = wait(NULL);
    assert(sing == rc);
    printf("(pid:%d) I am the parent of child:%d\n", getpid(), sing);
  }

  printf("(pid:%d) End\n", getpid());
  return EXIT_SUCCESS;
}

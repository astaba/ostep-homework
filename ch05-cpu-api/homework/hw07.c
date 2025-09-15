// Homework 5.07
// Write a program that creates a child process, and then in the child closes
// standard output (STDOUT FILENO). What happens if the child calls printf() to
// print some output after closing the descriptor?

// INFO:
// Since the descriptor is closed, there is no valid destination for the output.
// The printf() call silently fails in this case. By default, standard output
// writes fail without causing a crash.

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
    pid_t pid0 = getpid();
    printf("(pid:%d) Child process executing\n", pid0);

    close(STDOUT_FILENO);

    int bytes_nbr = printf("What happen now???\n");
    if (bytes_nbr < 0) {
      fprintf(stderr, "(pid:%d) ERROR: printf() failed: %s\n", pid0,
              strerror(errno));
    }

  } else {
    pid_t pid0 = getpid();
    printf("(pid:%d) Calling waitpid()...\n", pid0);
    int status = 0;
    pid_t pid1 = waitpid(rc, &status, 0);

    if (pid1 == -1) {
      perror("ERROR: waitpid() failed");
      exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status)) {
      printf("(pid:%d) Child:%d exit status: %d\n", pid0, pid1,
             WEXITSTATUS(status));
    } else {
      printf("(pid:%d) Child:%d exit status: abnormal\n", pid0, pid1);
    }

    assert(rc == pid1);
    printf("(pid:%d) fork()==%d\twaitpid()==%d\n", pid0, rc, pid1);
  }

  return EXIT_SUCCESS;
}

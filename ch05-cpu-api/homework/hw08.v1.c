/* ostep-homework/ch05-cpu-api/homework/hw08.v1.c */
// Homework 5.08
// Write a program that creates two children, and connects the standard output
// of one to the standard input of the other, using the pipe() system call.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  printf("(pid:%d) Parent: Hello, world!\n", getpid());

  int pipedes[2];

  if (pipe(pipedes) == -1) {
    perror("pipe() failed");
    exit(EXIT_FAILURE);
  }

  int pid1 = fork();

  if (pid1 < 0) {
    perror("fork() failed");
    exit(EXIT_FAILURE);

  } else if (pid1 == 0) {
    pid_t pid0 = getpid();
    printf("(pid:%d) Child 1 executing\n", pid0);

    close(pipedes[0]);
    char message[64];
    snprintf(message, sizeof(message),
             "(pid:%d) Hello this is Child 1 message.\n", pid0);
    write(pipedes[1], message, strlen(message));
    close(pipedes[1]);

    exit(EXIT_SUCCESS);
  }

  int pid2 = fork();

  if (pid2 < 0) {
    perror("fork() failed");
    exit(EXIT_FAILURE);

  } else if (pid2 == 0) {
    pid_t pid0 = getpid();
    printf("(pid:%d) Child 2 executing\n", pid0);

    close(pipedes[1]);

    char buffer[64];
    int byte_read = read(pipedes[0], buffer, sizeof(buffer));
    close(pipedes[0]);

    buffer[strcspn(buffer, "\n")] = '\0';
    printf("(pid:%d) Child 2 received: '%.*s'\n", pid0, byte_read, buffer);

    exit(EXIT_SUCCESS);
  }

  close(pipedes[0]);
  close(pipedes[1]);

  pid_t wpid1 = waitpid(pid1, NULL, 0);
  pid_t wpid2 = waitpid(pid2, NULL, 0);

  assert(pid1 == wpid1);
  assert(pid2 == wpid2);

  printf("(pid:%d) Both children have finished!\n", getpid());

  return EXIT_SUCCESS;
}

// Homework 5.06
// Write a slight modification of the previous program, this time using
// waitpid() instead of wait(). When would waitpid() be useful?

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {

  printf("(pid:%d) Hello, world!\n", getpid());

  pid_t rc = fork();

  if (rc < 0) {
    perror("fork() failed");
    exit(EXIT_FAILURE);

  } else if (rc == 0) {
    pid_t pid0 = getpid();
    printf("(pid:%d) Child process executing\n", pid0);
    for (int i = 0; i < 3; i++) {
      printf("(pid:%d) Let's sing again...\n", pid0);
      /* usleep(200000); */
    }

  } else {
    pid_t pid0 = getpid();
    printf("(pid:%d) Calling waitpid()...\n", pid0);
    int status = 0;
    pid_t pid1 = waitpid(rc, &status, 0);

    if (pid1 == -1) {
      perror("waitpid() failed");
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

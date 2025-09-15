// Homework 5.04 list
// Write a program that calls fork() and then calls some form of exec() to run
// the program /bin/ls. See if you can try all of the variants of exec(),
// including (on Linux) execl(), execle(), execlp(), execv(), execvp(), and
// execvpe(). Why do you think there are so many variants of the same basic
// call?

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {

  int rc = fork();

  if (rc < 0) {
    perror("fork() failed.\n");
    exit(EXIT_FAILURE);

  } else if (rc == 0) {
    close(STDOUT_FILENO);
    open("list.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    execl("/usr/bin/ls", "ls", "-A", (char *)NULL);

  } else {
    int ls = wait(NULL);
    assert(ls == rc);
  }

  return EXIT_SUCCESS;
}

/* ostep-homework/ch05-cpu-api/homework/hw04v.v0.c */
// Homework 5.04 vector path
// Write a program that calls fork() and then calls some form of exec() to run
// the program /bin/ls. See if you can try all of the variants of exec(),
// including (on Linux) execl(), execle(), execlp(), execv(), execvp(), and
// execvpe(). Why do you think there are so many variants of the same basic
// call?
// WARN: BAD CODE

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  int rc = fork();

  if (rc < 0) {
    perror("fork() failed.\n");
    exit(EXIT_FAILURE);

  } else if (rc == 0) {
    close(STDOUT_FILENO);
    open("list.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    // INFO:With this loop pass variadic args to the parent and the child
    // use exec??() to morph in a another process;
    // This illustrate how execv?() family functions are used to pass
    // a dynamic number of arguments unknown at compile time through
    // a dynamically allocated array.
    // BUG: This is the bad way. No memory is ever allocated even though
    // it works. Run with: valgrind --leak-check=full
    char **dynamargs = NULL;
    size_t i;
    for (i = 1; i < argc; i++) {
      char *tmp = realloc(dynamargs, sizeof(char *) + sizeof(dynamargs));
      if (!tmp) {
        perror("realloc() failde");
        exit(EXIT_FAILURE);
      } else {
        dynamargs = (char **)tmp;
      }
      dynamargs[i - 1] = strdup(argv[i]);
    }
    dynamargs[i - 1] = (char *)NULL;

    // char *argvector[3];
    // argvector[0] = strdup("/usr/bin/ls");
    // argvector[1] = strdup("-C");
    // argvector[2] = (char *)NULL;
    // execv(argvector[0], argvector);

    execv(dynamargs[0], dynamargs);

    free(dynamargs);

  } else {
    int ls = wait(NULL);
    assert(ls == rc);
  }

  return EXIT_SUCCESS;
}

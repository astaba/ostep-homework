/* ostep-homework/ch05-cpu-api/homework/hw04v.v1.c */
// Homework 5.04 vector path
// Write a program that calls fork() and then calls some form of exec() to run
// the program /bin/ls. See if you can try all of the variants of exec(),
// including (on Linux) execl(), execle(), execlp(), execv(), execvp(), and
// execvpe(). Why do you think there are so many variants of the same basic
// call?

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Check if any arguments were provided besides the program name
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <arg1> <arg2> ...\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("(pid:%d) Parent process starting.\n", getpid());

  // Fork a child process
  int rc = fork();
  if (rc < 0) {
    perror("fork() failed");
    exit(EXIT_FAILURE);
  }

  if (rc == 0) {
    // --- Child Process Logic ---
    printf("(pid:%d) Child process executing.\n", getpid());

    // We close stdout and redirect it to a file,
    // so the output of the new process goes there.
    close(STDOUT_FILENO);
    open("list.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    // Dynamically allocate an array to hold the arguments.
    // The size needs to be argc (for each argument) plus one for the
    // terminating NULL.
    char **dynamargs = malloc(sizeof(char *) * (argc));
    if (dynamargs == NULL) {
      perror("malloc() failed");
      exit(EXIT_FAILURE);
    }

    // Copy each argument from the parent's argv into our new array.
    // We start from argv[1] to skip the program name itself.
    for (int i = 1; i < argc; i++) {
      // strdup() allocates memory and copies the string.
      dynamargs[i - 1] = strdup(argv[i]);
      if (dynamargs[i - 1] == NULL) {
        perror("strdup() failed");
        // Clean up any previously allocated memory before exiting.
        for (int j = 0; j < i; j++) {
          free(dynamargs[j]);
        }
        free(dynamargs);
        exit(EXIT_FAILURE);
      }
    }
    // The last element must be a NULL pointer, as required by execv().
    dynamargs[argc - 1] = NULL;

    // This is where we call execv().
    // It will replace the current process with the new program.
    // The first argument to execv() is the program path.
    // The second argument is the dynamically created argument vector.
    execv(dynamargs[0], dynamargs);

    // The following lines will ONLY execute if execv() fails.
    // A common practice is to free memory here if execv() fails,
    // but since it replaces the process, this is for error cases only.
    perror("execv() failed to execute");
    // We free the memory we allocated only if execv() fails.
    // All elements but the last (char *)NULL where allocated with strdup()
    for (int i = 0; i < argc - 1; i++) {
      free(dynamargs[i]);
    }
    free(dynamargs);

    exit(EXIT_FAILURE);
  }

  // --- Parent Process Logic ---
  // Whether exec??() fails or not child process never get this far.
  int status = 0;
  // Wait for the child to finish and get its exit status.
  pid_t wpid = waitpid(rc, &status, 0);
  assert(wpid == rc);
  // Use the WIFEXITED and WEXITSTATUS macros to get the child's
  // exit information in a safe and portable way.
  if (WIFEXITED(status)) {
    printf("(pid:%d) Child exit status %d.\n", getpid(), WEXITSTATUS(status));
  } else {
    printf("(pid:%d) Child:%d exit status: abnormal\n", getpid(), wpid);
  }

  return EXIT_SUCCESS;
}

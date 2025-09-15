// Homework 5.03
// Write another program using fork(). The child process should print “hello”;
// the parent process should print “goodbye”. You should try to ensure that the
// child process always prints first; can you do this without calling wait() in
// the parent?
// NOTE: usleep() could be used in the parent process, however that would not be
// deterministic: Why? Process scheduling by the OS is not garanteed to honored
// the intended sequence. The delay usleep(200000) is arbitrary and
// system-dependent. If the system is heavily loaded the child process might
// still be preempted before printing "Hello".
// Use pipe() to send an arbitrary signal from child to parent and take
// advantage of the blocking nature of the read() function to pause the parent.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

  int pipefd[2]; // I/O fds

  if (pipe(pipefd) == -1) {
    perror("pipe() failed");
    exit(EXIT_FAILURE);
  }

  int rc = fork();

  if (rc < 0) {
    perror("fork() failed.");
    exit(EXIT_FAILURE);

  } else if (rc == 0) { // child (new process)
    close(pipefd[0]);
    printf("Child: Hello!\n");
    write(pipefd[1], "done", 4);
    close(pipefd[1]);

  } else { // parent goes down this path (original process)
    char buffer[4];
    close(pipefd[1]);
    read(pipefd[0], buffer, 4);
    close(pipefd[0]);
    printf("Parent: Goodbye!\n");
  }

  return EXIT_SUCCESS;
}

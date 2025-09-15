/* ostep-homework/ch05-cpu-api/homework/hw08.v0.c */
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
  // Print a message from the parent process with its PID
  printf("(pid:%d) Parent: Hello, world!\n", getpid());

  int pipe_fds[2]; // Array to hold the two I/O fds ends of the pipe

  if (pipe(pipe_fds) == -1) { // Create a pipe and handle any errors
    perror("pipe() failed");
    exit(EXIT_FAILURE);
  }

  int pid1 = fork(); // Fork the first child process

  if (pid1 < 0) { // Handle fork failure
    perror("fork() failed");
    exit(EXIT_FAILURE);

  } else if (pid1 == 0) { // Inside the first child process
    pid_t pid0 = getpid();
    printf("(pid:%d) Child 1 executing\n", pid0);

    close(pipe_fds[0]); // Close the read end of the pipe
    // Adjust stdout to refer pipe's write end fd
    dup2(pipe_fds[1], STDOUT_FILENO);
    close(pipe_fds[1]); // Close the write end of the pipe after duplication
    // Print a message that will be sent through the pipe
    printf("(pid:%d) Hello this is Child 1 message.\n", pid0);
    exit(EXIT_SUCCESS);
  }

  // Fork the second child process
  int pid2 = fork();

  if (pid2 < 0) { // Handle fork failure
    perror("fork() failed");
    exit(EXIT_FAILURE);

  } else if (pid2 == 0) { // Inside the second child process
    pid_t pid0 = getpid();
    printf("(pid:%d) Child 2 executing\n", pid0);

    close(pipe_fds[1]); // Close the write end of the pipe
    // Adjust stdin to refer pipe's read end fd
    dup2(pipe_fds[0], STDIN_FILENO);
    close(pipe_fds[0]); // Close the read end of the pipe after duplication

    char rd_buf[128]; // Buffer to store the message read from the pipe
    if (fgets(rd_buf, sizeof(rd_buf), stdin) == NULL) {
      perror("fgets() failed"); // Handle read failure
      exit(EXIT_FAILURE);
    };
    rd_buf[strcspn(rd_buf, "\n")] = '\0'; // Remove '\n' from the input

    // Print the message received from Child 1
    printf("(pid:%d) Child 2 received: '%s'\n", pid0, rd_buf);
    exit(EXIT_SUCCESS);
  }

  // Close both ends of the pipe in the parent process
  close(pipe_fds[0]);
  close(pipe_fds[1]);

  // Wait for both child processes to finish
  pid_t wpid1 = waitpid(pid1, NULL, 0);
  pid_t wpid2 = waitpid(pid2, NULL, 0);
  // Ensure that the correct PIDs are returned after the children finish
  assert(pid1 == wpid1);
  assert(pid2 == wpid2);
  // Print a message indicating both children have finished
  printf("(pid:%d) Both children have finished!\n", getpid());

  return EXIT_SUCCESS;
}

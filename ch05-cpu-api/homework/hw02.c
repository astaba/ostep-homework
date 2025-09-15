// Homework 5.02
// Write a program that opens a file (with the open() system call) and then
// calls fork() to create a new process. Can both the child and parent access
// the file descriptor returned by open()? What happens when they are writing to
// the file concurrently, i.e., at the same time?
// NOTE: Concluion
// In concurrent access both parent and child  share the same file descriptor,
// therefore system calls to write() modify the same shared offset (position
// indicator).
// As a result, without synchronization and depending on the scheduling, either:
// sequential execution concatenate the outputs or;
// Interleaved execution intangles the outputs.
// TEST: Try changing usleep argument and taking it in/out of for loops.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {

  int fd = open("boo.txt", O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd < 0) {
    perror("open() failed.");
    exit(EXIT_FAILURE);
  }
  printf("file descriptor: %d\n", fd);

  int rc = fork();

  if (rc < 0) {
    fprintf(stderr, "fork failed.\n");
    exit(EXIT_FAILURE);

  } else if (rc == 0) {
    /* char *child_msg = "Child writes\n"; */
    char child_msg[100];
    for (int i = 0; i < 5; i++) {
      snprintf(child_msg, 100, "\tChild writes (%d)\n", i + 1);
      int b8_written = write(fd, child_msg, strlen(child_msg));
      assert(b8_written == strlen(child_msg));
      // Small delay to simulate interleaving: "ChilParent writesd writes"
      usleep(100000);
    }

  } else {
    /* char *parent_msg = "Parent writes\n"; */
    char parent_msg[100];
    for (int i = 0; i < 5; i++) {
      snprintf(parent_msg, 100, "Parent writes (%d)\n", i + 1);
      int b8_written = write(fd, parent_msg, strlen(parent_msg));
      assert(b8_written == strlen(parent_msg));
    }
    // Small delay to simulate interleaving: "ChilParent writesd writes"
    usleep(100000);
  }

  close(fd);
  return EXIT_SUCCESS;
}

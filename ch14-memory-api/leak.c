/* ostep-homework/ch14-memory-api/leak.c */
// Created on: Wed Oct 22 13:15:33 +01 2025
// Homework 14.4:

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  char *ptr = malloc(128 * sizeof(char));
  if (!ptr) {
    perror("malloc() failed");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}

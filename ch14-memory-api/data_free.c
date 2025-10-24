/* ostep-homework/ch14-memory-api/data_free.c */
// Created on: Wed Oct 22 13:45:31 +01 2025
// Homework 14.6:

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int *data = (int *)malloc(sizeof(int) * 100);
  if (!data) {
    perror("malloc() failed");
    exit(EXIT_FAILURE);
  }

  free(data);

  printf("Deallocated (int) memory slot: %d\n", data[0]);

  return EXIT_SUCCESS;
}

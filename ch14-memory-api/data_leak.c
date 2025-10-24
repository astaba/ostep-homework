/* ostep-homework/ch14-memory-api/data_leak.c */
// Created on: Wed Oct 22 13:25:30 +01 2025
// Homework 14.5:

#include <stdlib.h>

int main(int argc, char *argv[]) {
  int data[100];

  data[100] = 0;

  return EXIT_SUCCESS;
}

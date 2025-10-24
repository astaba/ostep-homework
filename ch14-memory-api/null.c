/* ostep-homework/ch14-memory-api/null.c */
// Created on: Wed Oct 22 12:47:20 +01 2025
// Homework 14.1: First, write a simple program called null.c that creates a
// pointer to an integer, sets it to NULL, and then tries to dereference it.
// Compile this into an executable called null. What happens when you run this
// program

#include <stdlib.h>

int main(int argc, char *argv[]) {

  int *ptr = NULL;

  int n = *ptr;

  return EXIT_SUCCESS;
}

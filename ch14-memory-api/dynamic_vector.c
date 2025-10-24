/* ostep-homework/ch14-memory-api/dynamic_vector.c */
// Created on: Wed Oct 22 19:06:56 +01 2025
// Homework 14.8: Create a simple vector-like data structure and related
// routines that use realloc() to manage the vector. Use an array to store the
// vectors elements; when a user adds an entry to the vector, use realloc() to
// allocate more space for it.
// How well does such a vector perform? How does it compare to a linked list?
// Use valgrind to help you find bugs.

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// The vector-like data structure
typedef struct {
  int *data;
  size_t size;
  size_t cap;
} IntVector;

void vector_init(IntVector *v, size_t capacity) {
  capacity = (capacity == 0) ? 4 : capacity;
  // Allocate memory for the data
  v->data = (int *)malloc(capacity * sizeof(int));
  if (v->data == NULL) {
    perror("malloc() failed");
    exit(EXIT_FAILURE);
  }
  v->cap = capacity;
  v->size = 0;
}

void vector_push(IntVector *v, int value) {
  if (v->size == v->cap) {
    size_t new_cap = 2 * v->cap;

    // WARNING: COMMON AND DANGEROUS PITFALL !!!
    // The size passed to realloc() MUST be the total number of BYTES required.
    // Using only 'new_cap' (the element count) here would result in silent,
    // massive memory under-allocation. The program might not crash immediately
    // because the OS allocates memory in pages (e.g., 4096 bytes) and the
    // illegal writes fall into the unused yet legal 'slop' area. This leads to
    // subtle heap corruption only detectable with tools like Valgrind.

    int *tmp = realloc(v->data, new_cap * sizeof(int));

    if (tmp == NULL) {
      perror("realloc() failed");
      free(v->data);
      exit(EXIT_FAILURE);
    }

    v->data = tmp;
    v->cap = new_cap;
  }

  v->data[v->size] = value;
  v->size++;
}

void print_vector(IntVector *v) {
  if (v->data == NULL)
    return;
  printf("Vector (Size: %zu, Capacity: %zu): [", v->size, v->cap);
  for (size_t i = 0; i < v->size; i++) {
    printf("%d%s", v->data[i], (i < v->size - 1) ? ", " : "]\n");
  }
}

void free_vector(IntVector *v) {
  free(v->data);
  v->data = NULL;
  v->size = v->cap = 0;
}

int main(int argc, char *argv[argc + 1]) {
  // Declare int vector data structure
  IntVector vec;
  vector_init(&vec, 2);

  printf("Adding first element...\n");
  vector_push(&vec, 10); // Size: 1, Capacity: 2
  print_vector(&vec);

  printf("Adding second element...\n");
  vector_push(&vec, 20); // Size: 2, Capacity: 2
  print_vector(&vec);

  printf("Adding third element (Trigger resize to 4)...\n");
  vector_push(&vec, 30); // Size: 3, Capacity: 4
  print_vector(&vec);

  printf("Adding fourth element...\n");
  vector_push(&vec, 40); // Size: 4, Capacity: 4
  print_vector(&vec);

  printf("Adding fifth element (Trigger resize to 8)...\n");
  vector_push(&vec, 50); // Size: 5, Capacity: 8
  print_vector(&vec);

  free_vector(&vec);

  return EXIT_SUCCESS;
}

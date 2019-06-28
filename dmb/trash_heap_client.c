#include "./trash_heap.h"
#include <stdio.h>

void print_int(char c, int *x) {
  printf("%c: %p, *%c: %d\n", c, x, c, *x);
}

int main(int argc, char *argv[]) {
  int *x = (int *) get_trash(sizeof(int));
  print_int('x', x);
  *x = 42;

  int *y = (int *) get_trash(sizeof(int));
  *y = 43;
  print_int('x', x);
  print_int('y', y);

  free_trash(x);
  print_int('x', x);
  free_trash(y);
  printf("\n");
}

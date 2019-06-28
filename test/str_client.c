#include "../dmb/strs.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  assert(argc == 3);

  size_t n = 0;
  char **xs = split(argv[1], argv[2], &n);

  for (char **x = xs; *x; x++) {
    printf("%lu: %s\n", x - xs, *x);
  }
}

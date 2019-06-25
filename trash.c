#define MAX_LINE 1024
#define rstrip(s) s[strlen(s) - 1] = '\0'

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  char line[MAX_LINE];
  memset(line, 0, sizeof line);

  for (;;) {
    printf("> ");

    fgets(line, sizeof line, stdin);

    if (feof(stdin))
      break;

    rstrip(line);

    printf("%s (the trash heap has spoken)\n", line);
  }
  printf("\n");
}

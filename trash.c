#define MAX_LINE 1024
#define MAX_TOKENS 255

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// this is a little surprising
void lstrip(char **s) {
  while (isblank(**s))
    *s += 1;
}

void rstrip(char *s) {
  for (int i = strlen(s) - 1; i >= 0 && isblank(s[i]); i--)
    s[i] = '\0';
}

void strip(char *s) {
  lstrip(&s);
  rstrip(s);
}

int tokenize(char *s, char **tokens, size_t max_len) {
  memset(tokens, 0, max_len * sizeof(void *));
  int i = 0;
  for (char *token = strtok(s, " "); token != NULL; token = strtok(NULL, " " )) {
    if (i > MAX_TOKENS) {
      fprintf(stderr, "o_o\n");
      return -1;
    }

    tokens[i++] = token;
  }
  return i;
}

int main(int argc, char *argv[]) {
  printf("argv[1]: %s\n", argv[1]);

  char *tokens[MAX_TOKENS + 1];

  int err = tokenize(argv[1], tokens, MAX_TOKENS);

  for (char **t = tokens; *t != NULL; t++)
    printf("%ld: %s\n", t - tokens, *t);

  exit(0);


  char line[MAX_LINE];
  memset(line, 0, sizeof line);

  for (;;) {
    printf("> ");

    fgets(line, sizeof line, stdin);

    if (feof(stdin))
      break;

    strip(line);

    printf("%s (the trash heap has spoken)\n", line);
  }
  printf("\n");
}

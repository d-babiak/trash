#include "./strs.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// this is a little surprising
void lstrip(char **s) {
  while (isblank(**s))
    *s += 1;
}

void rstrip(char *s) {
  for (int i = strlen(s) - 1; i >= 0 && iswhitespace(s[i]); i--)
    s[i] = '\0';
}

void strip(char *s) {
  lstrip(&s);
  rstrip(s);
}

int tokenize(char *s, char **tokens, size_t max_tokens) {
  memset(tokens, 0, max_tokens * sizeof(void *));
  int i = 0;
  for (char *token = strtok(s, " "); token != NULL; token = strtok(NULL, " " )) {
    if (i > max_tokens) {
      fprintf(stderr, "o_o\n");
      return -1;
    }
    tokens[i++] = token;
  }
  return i;
}

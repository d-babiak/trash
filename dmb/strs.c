#include "./strs.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// this is a little surprising
void lstrip(char **s) {
  while (isblank(**s))
    *s += 1;
}

void rstrip(char *s) {
  for (int i = strlen(s) - 1; i >= 0 && iswhitespace(s[i]); i--)
    s[i] = '\0';
}

void strip(char **s) {
  lstrip(s);
  rstrip(*s);
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

char **split(char *s, char *key, size_t *n) {
  size_t n_chunks = 1, 
         len_s = strlen(s), 
         len_key = strlen(key);

  char *p1 = s, *p2;
  while (p1 < s + len_s && (p2 = strstr(p1, key))) { 
    p1 = p2 + len_key;
    n_chunks++;
  }

  char **chunks = calloc(n_chunks + 2, sizeof(void *));

  int i = 0;
  p1 = s;
  p2 = NULL;
  while (p1 < s + len_s && (p2 = strstr(p1, key))) { 
    memset(p2, 0, len_key);
    chunks[i++] = p1;
    p1 = p2 + len_key;
  }

  chunks[n_chunks - 1] = p1;

  *n = n_chunks;
  return chunks;
}

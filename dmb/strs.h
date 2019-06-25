#ifndef DMB_STRS_H
#define DMB_STRS_H
#include <sys/types.h>

#define MAX_TOKENS 255
#define streq(x,y) (x && y && !strcmp(x,y))
#define zero(xs)   memset(xs, 0, sizeof xs)

#define iswhitespace(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r')

void lstrip(char **s);

void rstrip(char *s);

void strip(char *s);

int tokenize(char *s, char **tokens, size_t max_len);
#endif

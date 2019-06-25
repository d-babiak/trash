#define MAX_LINE 1024
#define MAX_TOKENS 255
#define streq(x,y) (x && y && !strcmp(x,y))
#define zero(xs)   memset(xs, 0, sizeof xs)

#define iswhitespace(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r')

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void
__pwd() {
  char buf[4096];
  getcwd(buf, sizeof buf);
  printf("%s\n", buf);
}

const char *CMDS[] = {
  "clear",
  "ls",
  "rm",
  "touch",
  "vim",
  NULL
};

bool supported(char *cmd) {
  for (int i = 0; cmd[i]; i++)
    if (streq(CMDS[i], cmd))
      return true;

  return false;
}

void 
__eval(char **tokens, int n) {
  char *cmd = tokens[0];

  if (streq(cmd, "quit") || streq(cmd, "exit")) {
    printf("The trash heap has spoken\n");
    exit(0);
  }
  else if (streq(cmd, "pwd")) {
    __pwd();
  }
  else if (streq(cmd, "cd")) {
    char *dir = tokens[1];
    if (dir) // todo - fstat
      chdir(dir);
  }
  else if (supported(cmd)) {
    pid_t pid = fork();

    int wstatus = 0;

    bool background = false;
    if (streq(tokens[n - 1], "poof")) {
      background = true;
      tokens[n - 1] = NULL;
    } 

    switch (pid) {
      case -1: fprintf(stderr, "u_u\n"); 
        break;

      case  0: execvp(cmd, tokens);
        break;

      default: 
        if (background)
          return;
        else
          waitpid(pid, &wstatus, 0);
    }
  }
  else {
    for (char **t = tokens; *t; t++)
      printf("%ld: %s\n", t - tokens, *t);
  }
}


int main(int argc, char *argv[]) {
  char line[MAX_LINE];
  zero(line);
  char *tokens[MAX_TOKENS + 1];
  zero(tokens);

  for (;;) {
    printf("> ");

    fgets(line, sizeof line, stdin);

    if (feof(stdin))
      break;

    strip(line);
    int n = tokenize(line, tokens, MAX_TOKENS);

    if (n <= 0) 
      continue;

    __eval(tokens, n);
  }
  printf("\n");
}

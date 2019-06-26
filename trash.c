#include "./dmb/strs.h"
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 1024

void sig_handler(int n) {
  printf("o_o\n");
}

void install_signal_handlers() {
  struct sigaction *act = malloc(sizeof(struct sigaction));
  memset(act, 0, sizeof(struct sigaction));

  act->sa_handler = sig_handler;
  act->sa_flags   = 0;

  int err = sigaction(SIGINT, act, 0);
  assert(!err);
}

const char *CMDS[] = {
  "clear",
  "ls",
  "rm",
  "sleep",
  "touch",
  "vim",
  "python3.7",
  NULL
};

bool supported(char *cmd) {
  for (const char **s = CMDS; *s; s++)
    if (streq(*s, cmd))
      return true;

  return false;
}

void eval(char **tokens, int n) {
  char *cmd = tokens[0];

  if (streq(cmd, "quit") || streq(cmd, "exit")) {
    printf("The trash heap has spoken\n");
    exit(0);
  }
  else if (streq(cmd, "pwd")) {
    char buf[4096];
    getcwd(buf, sizeof buf);
    printf("%s\n", buf);
  }
  else if (streq(cmd, "cd")) {
    char *dir = tokens[1];
    if (dir) // todo - fstat
      chdir(dir);
  }
  else if (supported(cmd)) {
    int wstatus;

    bool background = streq(tokens[n - 1], "poof!");
    if (background)
      tokens[n - 1] = NULL;

    pid_t pid;
    switch (pid = fork()) {
      case -1: fprintf(stderr, "u_u\n"); 
        break;

      case  0: execvp(cmd, tokens);
        break;

      default: 
        if (!background)
          waitpid(pid, &wstatus, 0);
    }
  }
  else {
    for (char **t = tokens; *t; t++)
      printf("%ld: %s\n", t - tokens, *t);
  }
}

int main(int argc, char *argv[]) {
  install_signal_handlers();

  char line[MAX_LINE];
  char *tokens[MAX_TOKENS + 1];

  for (;;) {
    zero(line); 
    zero(tokens);
    printf("> ");

    fgets(line, sizeof line, stdin);

    if (feof(stdin))
      break;

    strip(line);
    int n = tokenize(line, tokens, MAX_TOKENS);

    if (n <= 0) 
      continue;

    eval(tokens, n);
  }

  printf("\n");
}

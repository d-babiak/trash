#include "./dmb/strs.h"
#include "./dmb/trash_heap.h"
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
  struct sigaction *act = get_trash(sizeof(struct sigaction));
  memset(act, 0, sizeof(struct sigaction));

  act->sa_handler = sig_handler;
  act->sa_flags   = 0;

  int err = sigaction(SIGINT, act, 0);
  assert(!err);
}

const char *CMDS[] = {
  "clear",
  "date",
  "false",
  "ls",
  "python3.7",
  "rm",
  "sleep",
  "touch",
  "true",
  "vim",
  NULL
};

bool supported(char *cmd) {
  for (const char **s = CMDS; *s; s++)
    if (streq(*s, cmd))
      return true;

  return false;
}

int eval(char **tokens, int n) {
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
        if (!background) {
          waitpid(pid, &wstatus, 0);
          return WEXITSTATUS(wstatus);
        }
    }
  }
  else {
    for (char **t = tokens; *t; t++)
      printf("%ld: %s\n", t - tokens, *t);
  }
  return 0;
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

    size_t n_cmds = 0;
    char **commands = split(line, "then", &n_cmds);
    for (char **cmd = commands; *cmd; cmd++)
      printf("%ld: %s\n", cmd - commands, *cmd);

    for (char **cmd = commands; *cmd; cmd++) {
      strip(*cmd);
      zero(tokens);
      int n = tokenize(*cmd, tokens, MAX_TOKENS);

      if (n <= 0)
        break;

      int cmd_err = eval(tokens, n);
      fprintf(stderr, "    (cmd: %s, cmd_err: %d)\n\n", tokens[0], cmd_err);
      if (cmd_err) 
        break;
    }
  }

  printf("\n");
}

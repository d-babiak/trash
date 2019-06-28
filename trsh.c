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
  "sort",
  "touch",
  "true",
  "uniq",
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

size_t n_pipes(char *line) {
  int N = 0;
  for (int i = 0; line[i]; i++)
    if (line[i] == '|')
      N++;
  return N;
}

void pipe_eval(char **cmds, size_t n) {
  pid_t pids[n];
  int pipe_prev[2];
  int pipe_next[2];

  for (size_t i = 0; i < n; i++) {
    bool has_upstream   = 0 < i,
         has_downstream =     i < n - 1;

    if (has_upstream) {
      pipe_prev[0] = pipe_next[0];
      pipe_prev[1] = pipe_next[1];
    }

    if (has_downstream)
      pipe(pipe_next);

    pid_t pid;

    switch (pid = fork()) {
      case -1: fprintf(stderr, "u_u\n"); 
        break;

      case  0: 
        if (has_upstream) {
          close(pipe_prev[STDOUT_FILENO]);
          dup2(pipe_prev[STDIN_FILENO], STDIN_FILENO);
        }

        if (has_downstream) {
          close(pipe_next[STDIN_FILENO]);
          dup2(pipe_next[STDOUT_FILENO], STDOUT_FILENO);
        }

        char *cmd = cmds[i]; 
        char *tokens[MAX_TOKENS + 1];
        zero(tokens);
        tokenize(cmd, tokens, MAX_TOKENS);
        execvp(cmd, tokens);
        break;

      default: 
        pids[i] = pid;
        if (has_upstream) {
          close(pipe_prev[0]);
          close(pipe_prev[1]);
        }
    }

    for (int i = 0; i < n; i++) {
      int status;
      waitpid(pids[i], &status, 0);
    }
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

    size_t n_cmds = 0;

    size_t num_pipes = n_pipes(line);
    if (num_pipes > 0) {
      char **commands = split(line, "|", &n_cmds);

      for (char **cmd = commands; *cmd; cmd++)
        strip(cmd);

      pipe_eval(commands, n_cmds);
    } 
    else {
      char **commands = split(line, "&&", &n_cmds);

      for (char **cmd = commands; *cmd; cmd++)
        fprintf(stderr, "%ld: %s\n", cmd - commands, *cmd);

      for (char **cmd = commands; *cmd; cmd++) {
        strip(cmd);
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

  }

  printf("\n");
}

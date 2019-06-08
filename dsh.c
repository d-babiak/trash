#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int child_proc(const char *cmd) {
  pid_t pid = fork();
  assert(pid >= 0);
  if (pid == 0) {
    char *const argv[] = {"ls", NULL};
    execv(cmd, argv);
  } else {
    int status;
    waitpid(pid, &status, 0);
  }
}

bool is_whitespace(char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

bool streq(const char *s1, const char *s2) {
  return (!s1 && !s2) || (s1 && s2 && (0 == strcmp(s1, s2)));
}

char *rstrip(char *s) {
  int n = strlen(s) - 1;
  while (n >= 0 && is_whitespace(s[n]))
    s[n--] = '\0';
  return s;
}

bool startswith(char *target, char *prefix) {
  size_t N = strlen(prefix);
  if (strlen(target) < N)
    return false;

  for (int i = 0; i < N; i++)
    if (target[i] != prefix[i])
      return false;

  return true;
}

void _pwd() {
  char buf[256];
  printf("%s\n", getcwd(buf, sizeof buf));
}

void _cd(const char *cmd) {
  char path[256];
  memset(path, 0, sizeof path);
  int n = sscanf(cmd, "cd %s", path);
  assert(n == 1);
  int err = chdir(path);
  assert(!err);
  printf("err: %d\n", err);
}

void _ls() {
  child_proc("/usr/bin/ls");
}

void exec_cmd(char *cmd) {
  rstrip(cmd);
  if (streq(cmd, "pwd")) 
    _pwd();
  else if (startswith(cmd, "cd "))
    _cd(cmd);
  else if (streq(cmd, "ls"))   
    _ls();
  else
    printf("wtf: %s\n", cmd);
}

void prompt() {
  char buf[256];
  getcwd(buf, sizeof buf);
  printf("[%s]\n> ", buf);
}

int main(int argc, char *argv[]) {
  char line[4096];
  memset(line, 0, sizeof line);

  setlinebuf(stdin);


  for (;;) {
    prompt();

    if (NULL == fgets(line, sizeof line, stdin)) 
      break;

    exec_cmd(line);
  }
  printf("\n");
}

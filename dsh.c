#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int child_proc(const char *cmd) {
  pid_t pid = fork();
  assert(pid >= 0);

  if (pid == 0) {
    char buf[4096];
    memcpy(buf, cmd, sizeof buf);

    char prog_path[256];
    memset(prog_path, 0, sizeof prog_path);
    strcat(prog_path, "/usr/bin/");

    char *prog_name = strtok(buf, " "); 
    strcat(prog_path, prog_name);

    char *argv[] = {prog_name, NULL};

    char *stdin_redirect; 
    char *stdout_redirect; 

    for (char *s = strtok(NULL, " "); s != NULL; s = strtok(NULL, " ")) {
      if (*s == '>')
        stdout_redirect = s + 1;

      if (*s == '<')
        stdin_redirect = s + 1;
    }

    if (stdout_redirect) {
      errno = 0;
      int fd = open(stdout_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (errno)
        perror("open for write");

      assert(fd >= 0) ;

      printf("%s %d\n", __FILE__, __LINE__);

      errno = 0;
      int new_fd = dup2(fd, STDOUT_FILENO);

      if (errno)
        perror("dup2");

      assert(new_fd == STDOUT_FILENO);

      int err = close(fd);
      assert(!err);
    }

    if (stdin_redirect) {
      int fd = open(stdin_redirect, O_RDONLY);
      assert(fd >= 0) ;

      int err = dup2(fd, STDOUT_FILENO);
      assert(!err);

      err = close(fd);
      assert(!err);
    }

    execv(prog_path, argv);
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

char *strip(char **s) {
  while (is_whitespace(**s)) {
    *s += 1;
  }
  return rstrip(*s);
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

void exec_cmd(char *cmd) {
  rstrip(cmd);
  if (startswith(cmd, "cd "))
    _cd(cmd);

  else 
    child_proc(cmd);
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

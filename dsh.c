#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

bool streq(const char *s1, const char *s2) {
  return (!s1 && !s2) || (s1 && s2 && (0 == strcmp(s1, s2)));
}

void _pwd() {
  char buf[256];
  printf("%s\n", getcwd(buf, sizeof buf));
}

void exec_cmd(char *cmd) {
  if (streq(cmd, "pwd"))
    _pwd();
}

bool is_whitespace(char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

char *rstrip(char *s) {
  int n = strlen(s) - 1;
  while (n >= 0 && is_whitespace(s[n]))
    s[n--] = '\0';
  return s;
}

int main(int argc, char *argv[]) {
  char line[4096];
  memset(line, 0, sizeof line);

  setlinebuf(stdin);


  for (;;) {
    printf("> ");

    if (NULL == fgets(line, sizeof line, stdin)) 
      break;

    exec_cmd(rstrip(line));
    //fputs(line, stdout);
    //fputs(line, stderr);
  }
  printf("\n");
}

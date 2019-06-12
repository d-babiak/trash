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


typedef struct __Proc {
  //char  prog_name[256];
  char  *prog_name;
  int   in_fd;
  int   out_fd;
  pid_t pid;
} Proc;

void print_proc(Proc *p) {
  if (p->prog_name == NULL) {
    fprintf(stderr, "wtf wtf wtf\n");
  }
  fprintf(stderr, "prog: %s",   p->prog_name);
  fprintf(stderr, " in: %d",    p->in_fd);
  fprintf(stderr, " out: %d\n", p->out_fd);
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

char *strip(char *s) {
  while (is_whitespace(*s)) {
    s += 1;
  }
  return rstrip(s);
}

int char_count(const char *s, char c) {
  int N = 0; 
  while (*s) 
    if (*s++ == c)
      N++;
  return N;
}

int len(void **xs) {
  printf("          %s start\n", __func__);
  int N = 0;
  while (*xs++ != NULL)
    N++;
  printf("          %s end\n", __func__);
  return N;
}

void run_procs(Proc **procs) {
  int N = len((void **) procs);
  fprintf(stderr, "line %d, N: %d\n", __LINE__, N);

  pid_t pid;    

  for (int i = 0; i < N; i++) {
    Proc *p = procs[i];

    assert(0 <= (pid = fork()));

    if (pid > 0) {
      // shell process
      p->pid = pid;
      continue;
    }

    if (i > 0) {
      // The Linux Programming Interface "Closing unused pipe file descriptors" 
      // Ch44, pg 894 Michael Kerrisk :+1:
      int err = close(procs[i - 1]->out_fd);
      assert(!err);
    }

    fprintf(stderr, "  CHILD: ");
    print_proc(p);
    
    bool ok;
    if (p->in_fd != STDIN_FILENO) {
      ok = (0 <= dup2(p->in_fd, STDIN_FILENO));

      if (!ok) {
        perror("STDIN_FILENO"); exit(errno);
      }
    }

    if (p->out_fd != STDOUT_FILENO) {
      ok = (0 <= dup2(p->out_fd, STDOUT_FILENO));

      if (!ok) {
        perror("STDOUT_FILENO"); exit(errno);
      }
    }

    char prog_path[256];
    memset(prog_path, 0, sizeof prog_path);
    strcat(prog_path, "/usr/bin/");
    strcat(prog_path, p->prog_name);
    char *argv[] = {p->prog_name, NULL};

    fprintf(stderr, "  CHILD: %s\n", prog_path);

    int rc = execv(prog_path, argv);
    if (rc > -1)
      perror("execv");
  }

  if (pid > 0) {
    // shell process
    for (int i = 0; i < N; i++) {
      int err = close(procs[i]->in_fd);
      assert(!err);
      err = close(procs[i]->out_fd);
    }
    for (int i = 0; i < N; i++) {
      int status;
      waitpid(procs[i]->pid, &status, 0);
    }
  }
}

Proc **parse_procs(const char *line) {
  char buf[4096];
  strcpy(buf, line);

  int N = char_count(line, '|') + 1;

  size_t size = (N + 1) * sizeof(void *);
  Proc **procs = malloc(size);
  memset((void *) procs, 0, size);

  int rw[2] = {STDIN_FILENO, STDOUT_FILENO};
  int err = 0;

  char *strs[N]; int i = 0;
  for (char *cmd = strtok(buf, "|"); cmd != NULL; cmd = strtok(NULL, "|"))
    strs[i++] = cmd;

  for (i = 0; i < N; i++) {
    char *cmd = strs[i];
    printf("i: %d, cmd: %s\n", i, cmd);
    Proc *proc = malloc(sizeof(Proc));
    proc->in_fd = STDIN_FILENO;
    proc->out_fd = STDOUT_FILENO;
    //strcpy(proc->prog_name, "NULL\0");
    proc->prog_name = NULL;
    proc->pid = 0;

    proc->in_fd = rw[0];

    if (1 < N && i < N - 1) {
      assert(!(err = pipe(rw)));
      proc->out_fd = rw[1];
    }

    char *prog_name = strip(strdup(strtok(cmd, " ")));
    printf("  LINE: %d, prog_name (%s)\n", __LINE__, prog_name);
    proc->prog_name = prog_name;

    if (i == N - 1) {
      for (char *s = strtok(NULL, " "); s != NULL; s = strtok(NULL, " ")) {
        if (*s == '>') {
          proc->out_fd = open(s + 1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          assert(proc->out_fd >= 0);
        }
      }
    }
    procs[i] = proc;
    //printf("LINE %d %s\n", __LINE__, "assigning to struct");
    /*
    Proc **FFS = (&procs);
    Proc **addr = FFS + i;
    printf("      addr: %p\n", addr);
    *addr = proc;
    print_proc(*addr);
    printf("  LINE %d %s\n", __LINE__, "assigned to struct");
    */
  }
  printf("N: %d\n", N);
  for (int j = 0; j < N; j++)
    print_proc(procs[j]);
  return procs;
}

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

      assert(fd >= 0);

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

  else {
    Proc **procs = parse_procs(cmd);
    run_procs(procs);
  }
    //child_proc(cmd);
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

int *pipe_prev = NULL;
int *pipe_next = malloc(2 * sizeof(int));

// calls to pipe are only ever made by the shell

if (/*there are downstream processes*/)
  pipe(pipe_next);

pid pid = fork()

assert(pid_1 >= 0);

if (child) {
  if (/*has upstream*/) {
    close();
    dup2();
  }

  if (/*has downstream*/) {
    close();
    dup2();
  }
  exec(...);
} 
else {
  if (pipe_prev) {
    close(pipe_prev[0]);
    close(pipe_prev[1]);
  }
  pipe_prev = pipe_next;
}


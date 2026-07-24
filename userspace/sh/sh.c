#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>

void
parse_and_execute(const char *buf, size_t n)
{
  char cmd[64] = "/boot/";

  // extract the first word
  for (size_t i = 0; i < n; i++)
  {
    if (buf[i] == ' ' || buf[i] == '\n')
    {
      cmd[6+i] = '\0';
      break;
    }
    cmd[6+i] = buf[i];
  }

  // execute it as a child process
  int pid = fork();
  if (pid != 0)
  {
    // set child as the foreground process
    set_fg_proc(pid);
    // wait for child to finish
    wait(NULL);
  }
  else
  {
    set_fg_proc(getpid());
    if (execve(cmd, 0, 0) < 0) {
      printf("not found: %s\n", cmd);
      exit(1);
    }
  }
}

int main()
{
  char buf[512];
  ssize_t n;
  int pid = getpid();

  set_fg_proc(pid);

  while (1)
  {
    // write prompt to stdout
    printf("> ");

    // read user input
    n = read(0, buf, 512);

    // try executing the input
    if (n > 0)
    {
      parse_and_execute(buf, n);

      /* here the child either finishes execution, gets
       * interrupted (CTR+C) or execve failed. */
      set_fg_proc(pid);
    }
  }

  return 0;
}

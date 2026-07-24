#include <unistd.h>
#include <sys/wait.h>

int main()
{
  int pid = fork();
  if (pid == 0)
  {
    // launch the shell program
    execve("/boot/sh", 0, 0);
  }
  else
  {
    // reap zombie childs with no parent
    while (1)
      wait(NULL);
  }

  return 0;
}

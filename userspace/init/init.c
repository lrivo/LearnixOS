#include <liblearnix.h> 

char msg[] = "Hello from the init process\n";

int main()
{
  // write welcome message on stdout
  write(1, msg, 29);
  
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
    {
      wait();
    }
  }
  
  return 0;
}

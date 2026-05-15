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
    // parent spin wait
    for (;;)
      ;
  }
  
  return 0;
}

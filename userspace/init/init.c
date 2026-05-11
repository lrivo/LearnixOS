#include <liblearnix.h> 

char msg[] = "Hello from the init process\n";
char parent[] = "parent\n";
char child[] = "child\n";

int main()
{
  // write welcome message on stdout
  write(1, msg, 29);
  
  int pid = fork();
  if (pid == 0)
  {
    // launch the hello program
    execve("/boot/hello", 0, 0);
  }
  else
  {
    // parent
    for (;;)
      write(1, parent, 8);
  }
  
  return 0;
}

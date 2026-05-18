#include <liblearnix.h>
#include <string.h>

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
    wait();
  }
  else
  {
    if (execve(cmd, 0, 0) < 0)
      write(1, "error\n", 6);
  }
}

int main()
{
  char buf[512];
  ssize_t n;
  
  while (1)
  {
    // write prompt to stdout
    write(1, "> ", 2);
    
    // read user input
    n = read(0, buf, 512);
   
    // echo back to stdout
    if (n > 0)
    {
      parse_and_execute(buf, n);
    }
  }

  return 0;
}

#include <liblearnix.h>

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
      write(1, "echo: ", 6);
      write(1, buf, n); 
    }
  }

  return 0;
}

#include <liblearnix.h>

const char msg[] = "Hello World!\n";

int main()
{
  if (getpid() == 2)
  {
    for (;;)
      write(1, msg, 14);
  }
  else
    write(1, msg, 5);

  return 0;
}

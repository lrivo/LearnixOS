#include <liblearnix.h>

const char msg[] = "Hello World!\n";

int main()
{
  for (;;)
    write(1, msg, 14);

  return 0;
}

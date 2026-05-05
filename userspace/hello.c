#include <liblearnix.h> 

int main()
{
  char hello[] = "Hello from C userspace\n";

  for (int i = 0; i < 10; i++)
    write(1, hello, 24);

  return 0;
}

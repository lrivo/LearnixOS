#include <liblearnix.h>
#include <string.h>
#include <stdbool.h>

void *shared = (void*)0x500000;

/*
 * NOTE: after 2/3 executions both parent and child always print "-> !"
 * meaning that the underlying mmap mapping is re-used even after
 * both have been killed.
 * And that is strange given that mmap always memset(0) the physical page.
 */
int main()
{
  int pid = fork();
  if (pid == 0)
  {
    write(1, "child\n", 6);
    char* s = mmap(shared, 4096, PROT_WRITE, MAP_SHARED);
    write(1, "-> ", 2);
    write(1, s, 1);
    write(1, "\n", 1);
    if (s[0] != '!')
    {
      s[0] = '!';
      memset(&s[1], 0x42, 10);
      write(1, "cif\n", 4);
      write(1, s, 11);
    }
    else
    {
      write(1, "celse\n", 6);
      write(1, s, 11);
    }
  }
  else
  {
    write(1, "parent\n", 7);
    char* s = mmap(shared, 4096, PROT_WRITE, MAP_SHARED);
    write(1, "-> ", 2);
    write(1, s, 1);
    write(1, "\n", 1);
    if (s[0] != '!')
    {
      s[0] = '!';
      memset(&s[1], 0x41, 10);
      write(1, "pif\n", 4);
      write(1, s, 11);
    }
    else
    {
      write(1, "pelse\n", 6);
      write(1, s, 11);
    }
    wait();
  }

  munmap(shared, 4096);

  return 0;
}

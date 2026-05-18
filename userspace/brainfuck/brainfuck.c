#include <liblearnix.h>

uint8_t memory[2048] = { 0 };
uint8_t program[1024];

static inline uint8_t*
find_prev_bracket(uint8_t *ip)
{
  uint8_t *curr = ip;
  while (curr >= program && *curr != '[') 
    ;
  return curr;
}

int main()
{
  // read the program text from stdin
  int n = read(0, (char*)program, 1024);
  if (n == 0)
    exit(1);

  // interpret it
  uint8_t *ip = program, *dp = memory;
  while (1)
  {
    // evaluate ip's value before incrementing it
    switch (*ip)
    {
      case '>':
	++dp; break;
      case '<':
	--dp; break;
      case '+':
	++(*dp); break;
      case '-':
	--(*dp); break;
      case '.':
	write(1, (char*)dp, 1); break;
      case ',':
	read(0, (char*)dp, 1); break;
      case '[':
	break;
      case ']':
	ip = find_prev_bracket(ip); break;
      default:
	exit(-1);
    }
    ip++;
  }
  write(1, "done\n", 5);
  return 0;
}

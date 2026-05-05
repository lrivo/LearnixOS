#include "include/liblearnix.h"

long int
write(int fd, char *buf, size_t len)
{
  return (long int)_do_syscall(SYS_WRITE, fd, (size_t)buf, len, 0, 0, 0);
}

__attribute__((noreturn)) void
exit(size_t code)
{
  _do_syscall(SYS_EXIT, code, 0, 0, 0, 0, 0);
}

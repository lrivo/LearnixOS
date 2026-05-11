#include "include/liblearnix.h"

long int
write(int fd, char *buf, size_t len)
{
  return (long int)_do_syscall(SYS_WRITE, fd, (size_t)buf, len, 0, 0, 0);
}

int
fork()
{
  return (int)_do_syscall(SYS_FORK, 0, 0, 0, 0, 0, 0);
}

int
execve(const char *path, const char **argv, const char **envp)
{
  return (int)_do_syscall(SYS_EXECVE, (size_t)path, (size_t)argv, (size_t)envp, 0, 0, 0);
}

__attribute__((noreturn)) void
exit(size_t code)
{
  _do_syscall(SYS_EXIT, code, 0, 0, 0, 0, 0);
}

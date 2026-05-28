#include "include/liblearnix.h"
#include <stdint.h>

long int
read(int fd, char *buf, size_t len)
{
  return (long int)_do_syscall(SYS_READ, fd, (size_t)buf, len, 0, 0, 0);
}

long int
write(int fd, char *buf, size_t len)
{
  return (long int)_do_syscall(SYS_WRITE, fd, (size_t)buf, len, 0, 0, 0);
}

void*
mmap(void *addr, size_t length, int prot, int flags)
{
  return (void*)_do_syscall(SYS_MMAP, (size_t)addr, length, (size_t)prot, (size_t)flags, 0, 0);
}

int
munmap(void *addr, size_t length)
{
  return (int)_do_syscall(SYS_MUNMAP, (size_t)addr, length, 0, 0, 0, 0);
}

int32_t
getpid(void)
{
  return (int32_t)_do_syscall(SYS_GETPID, 0, 0, 0, 0, 0, 0);
}

int
fork(void)
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

long int
wait(void)
{
  return (long int)_do_syscall(SYS_WAIT, 0, 0, 0, 0, 0, 0);
}

int
set_fg_proc(uint32_t pid)
{
  return (int)_do_syscall(SYS_SET_FG_PROC, pid, 0, 0, 0, 0, 0);
}

/* strings.h */
int
strcmp(const char *s1, const char *s2)
{
  while (*s1 && (*s1 == *s2))
  {
    s1++; s2++;
  }
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void *
memcpy (void *restrict dest, const void *restrict src, size_t n)
{
  asm volatile ("rep movsb" : "+D"(dest), "+S"(src), "+c"(n)::"memory");
  return dest;
}

void *
memset (void *s, int c, size_t n)
{
  asm volatile ("rep stosb" ::"D"(s), "a"(c), "c"(n) : "cc", "memory");
  return s;
}

/* stack canaries */

/* this is the function called when the canary is overwritten */
__attribute__((noreturn))
void __stack_chk_fail(void)
{
  write(1, "stack smashing detected\n", 24); 
  exit(-1);
}

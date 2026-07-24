#include <stddef.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

ssize_t
read(int fd, const void *buf, size_t len) {
  return (ssize_t)_do_syscall(SYS_READ, fd, (size_t)buf, len, 0, 0, 0);
}

ssize_t
write(int fd, const void *buf, size_t len) {
  return (ssize_t)_do_syscall(SYS_WRITE, fd, (size_t)buf, len, 0, 0, 0);
}

int
open(const char *pathname, int flags, int mode) {
    return (int)_do_syscall(SYS_OPEN, (size_t)pathname, (size_t)flags, (size_t)mode, 0, 0, 0);
}

int
close(int fd) {
    return (int)_do_syscall(SYS_CLOSE, (size_t)fd, 0, 0, 0, 0, 0);
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

int
pipe(int pipefd[2]) {
  return (int)_do_syscall(SYS_PIPE, (size_t)pipefd, 0, 0, 0, 0, 0);
}

int
yield(void) {
    return (int)_do_syscall(SYS_YIELD, 0, 0, 0, 0, 0, 0);
}

pid_t
getpid(void) {
  return (int32_t)_do_syscall(SYS_GETPID, 0, 0, 0, 0, 0, 0);
}

pid_t
getppid(void) {
  return (int32_t)_do_syscall(SYS_GETPPID, 0, 0, 0, 0, 0, 0);
}

pid_t
fork(void) {
  return (int)_do_syscall(SYS_FORK, 0, 0, 0, 0, 0, 0);
}

int
execve(const char *path, const char **argv, const char **envp) {
  return (int)_do_syscall(SYS_EXECVE, (size_t)path, (size_t)argv, (size_t)envp, 0, 0, 0);
}

__attribute__((noreturn)) void
exit(int status) {
  _do_syscall(SYS_EXIT, (size_t)status, 0, 0, 0, 0, 0);
}

pid_t
wait(int *wstatus) {
  return (pid_t)_do_syscall(SYS_WAIT, (size_t)wstatus, 0, 0, 0, 0, 0);
}

pid_t
waitpid(pid_t pid, int *wstatus, int options) {
    // TODO: wait4 not yet implemented kernel level
    return wait(wstatus);
}

int
set_fg_proc(pid_t pid) {
  return (pid_t)_do_syscall(SYS_SET_FG_PROC, pid, 0, 0, 0, 0, 0);
}

/* stdio.h */
void _putchar(char character) {
    // TODO: will need some sort of buffering
    write(1, &character, 1);
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
// this is the function called when the canary is overwritten
__attribute__((noreturn))
void __stack_chk_fail(void)
{
  write(1, "stack smashing detected\n", 24);
  exit(1);
}

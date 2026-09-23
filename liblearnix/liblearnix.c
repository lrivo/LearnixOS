#include <stddef.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sched.h>

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

off_t
lseek(int fd, off_t offset, int whence) {
  return (off_t)_do_syscall(SYS_LSEEK, (size_t)fd, (size_t)offset, (size_t)whence, 0, 0, 0);
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
dup(int oldfd) {
    return (int)_do_syscall(SYS_DUP, (size_t)oldfd, 0, 0, 0, 0, 0);
}

int
dup2(int oldfd, int newfd) {
    return (int)_do_syscall(SYS_DUP2, (size_t)oldfd, (size_t)newfd, 0, 0, 0, 0);
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
    return waitpid(-1, wstatus, 0);
}

pid_t
waitpid(pid_t pid, int *wstatus, int options) {
    return (pid_t)_do_syscall(SYS_WAIT, (size_t)pid, (size_t)wstatus, (size_t)options, 0, 0, 0);
}

int
set_fg_proc(pid_t pid) {
  return (pid_t)_do_syscall(SYS_SET_FG_PROC, pid, 0, 0, 0, 0, 0);
}

int
sched_get_stats(struct sched_stats *stats) {
  return (int)_do_syscall(SYS_SCHED_GET_STATS, (size_t)stats, 0, 0, 0, 0, 0);
}

int
sched_set_prio(pid_t pid, unsigned int prio) {
  return (int)_do_syscall(SYS_SCHED_SET_PRIO, (size_t)pid, (size_t)prio, 0, 0, 0, 0);
}

/* stdio.h */
void _putchar(char character) {
    // TODO: buffering
    write(1, &character, 1);
}

/* stack canaries */
// this is the function called when the canary is overwritten
__attribute__((noreturn))
void __stack_chk_fail(void)
{
  write(1, "stack smashing detected\n", 24);
  exit(1);
}

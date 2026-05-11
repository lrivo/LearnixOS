#pragma once
#include <stddef.h>
#include <stdint.h>

#define SYS_WRITE 1
#define SYS_GETPID 39
#define SYS_FORK 57
#define SYS_EXECVE 59
#define SYS_EXIT 60

/* SYSCALLS */
long int write(int fd, char *buf, size_t len);
int32_t getpid(void);
int fork(void);
int execve(const char *path, const char **argv, const char **envp);
void exit(size_t code);

// NASM macro that actually does the syscall
extern size_t _do_syscall(size_t number, size_t arg0, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5);

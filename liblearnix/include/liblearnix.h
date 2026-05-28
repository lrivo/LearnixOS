#pragma once
#include <stddef.h>
#include <stdint.h>

typedef int64_t ssize_t;

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_MMAP 9
#define SYS_MUNMAP 11
#define SYS_GETPID 39
#define SYS_FORK 57
#define SYS_EXECVE 59
#define SYS_EXIT 60
#define SYS_WAIT 61
#define SYS_SET_FG_PROC 62

/* SYSCALLS */
long int read(int fd, char *buf, size_t len);
long int write(int fd, char *buf, size_t len);

#define PROT_NONE (1 << 0U)
#define PROT_READ (1 << 1U)
#define PROT_WRITE (1 << 2U)
#define PROT_EXEC (1 << 3U)
#define MAP_PRIVATE 0
#define MAP_SHARED 1
void* mmap(void *addr, size_t length, int prot, int flags);
int munmap(void *addr, size_t length);

int32_t getpid(void);
int fork(void);
int execve(const char *path, const char **argv, const char **envp);
void exit(size_t code);
long int wait(void);
int set_fg_proc(uint32_t pid);

// NASM macro that actually does the syscall
extern size_t _do_syscall(size_t number, size_t arg0, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5);

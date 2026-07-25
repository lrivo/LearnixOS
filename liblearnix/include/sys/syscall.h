#pragma once
#include <sys/types.h>

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_MMAP 9
#define SYS_MUNMAP 11
#define SYS_PIPE 22
#define SYS_YIELD 24
#define SYS_DUP 32
#define SYS_DUP2 33
#define SYS_GETPID 39
#define SYS_GETPPID 40
#define SYS_FORK 57
#define SYS_EXECVE 59
#define SYS_EXIT 60
#define SYS_WAIT 61
#define SYS_SET_FG_PROC 62

int set_fg_proc(pid_t pid);

// NASM macro (liblearnix.asm) that performs a syscall
extern size_t _do_syscall(size_t number, size_t arg0, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5);

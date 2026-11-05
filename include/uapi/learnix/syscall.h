#pragma once

/* Maximum path length accepted by path-taking syscalls (open, execve) */
#define PATH_MAX 128

/* Linux syscall numbers */
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_LSEEK 8
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
#define SYS_SCHED_GET_STATS 63
#define SYS_SCHED_SET_PRIO 64
#define SYS_PMM_STATS_START 65
#define SYS_PMM_STATS_GET 66

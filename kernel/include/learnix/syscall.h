#pragma once
#include <learnix/arch/interrupts.h>
#include <learnix/types.h>

/* Linux syscall errors (they are returned negated) */
#define EBADF 9     // bad file descriptor
#define EFAULT 14   // invalid address
#define EEXIST 17   // already exist
#define EINVAL 22
#define EPIPE 32    // broken pipe
#define EPERM 1     // operation not permitted
#define ENOTSUP 95  // operation not supported

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

/* This C function gets called by the syscall_entry assembly
 * routine for each HW architecture. */
void syscall_dispatcher (struct intr_trap_frame *tf);

/* Actual syscalls */
void sys_read (struct intr_trap_frame *tf);
void sys_write (struct intr_trap_frame *tf);
void sys_open (struct intr_trap_frame *tf);
void sys_close (struct intr_trap_frame *tf);

#define SEEK_SET 0
#define SEEK_CURR 1
#define SEEK_END 2
void sys_lseek (struct intr_trap_frame *tf);

#define PROT_NONE (1 << 0U)
#define PROT_READ (1 << 1U)
#define PROT_WRITE (1 << 2U)
#define PROT_EXEC (1 << 3U)
#define MAP_PRIVATE 0
#define MAP_SHARED 1
void sys_mmap (struct intr_trap_frame *tf);
void sys_munmap (struct intr_trap_frame *tf);

void sys_yield (struct intr_trap_frame *tf);
void sys_dup (struct intr_trap_frame *tf);
void sys_dup2 (struct intr_trap_frame *tf);
void sys_pipe (struct intr_trap_frame *tf);
void sys_getpid (struct intr_trap_frame *tf);
void sys_getppid (struct intr_trap_frame *tf);
void sys_fork (struct intr_trap_frame *tf);
void sys_execve (struct intr_trap_frame *tf);
void sys_exit (struct intr_trap_frame *tf);
void sys_wait (struct intr_trap_frame *tf);

/* Custom syscalls */
void sys_set_fg_proc (struct intr_trap_frame *tf);
void sys_sched_get_stats(struct intr_trap_frame *tf);
void sys_sched_set_prio(struct intr_trap_frame *tf);
void sys_pmm_stats_start(struct intr_trap_frame *tf);
void sys_pmm_stats_get(struct intr_trap_frame *tf);
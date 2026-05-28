#pragma once
#include <learnix/arch/interrupts.h>
#include <learnix/types.h>

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

/* This C function gets called by the syscall_entry assembly
 * routine for each HW architecture. */
void syscall_dispatcher (struct intr_trap_frame *tf);

/* Linux syscalls */
void sys_read (struct intr_trap_frame *tf);

#define PROT_NONE (1 << 0U)
#define PROT_READ (1 << 1U)
#define PROT_WRITE (1 << 2U)
#define PROT_EXEC (1 << 3U)
#define MAP_PRIVATE 0
#define MAP_SHARED 1
void sys_mmap (struct intr_trap_frame *tf);

void sys_munmap (struct intr_trap_frame *tf);

void sys_write (struct intr_trap_frame *tf);
void sys_getpid (struct intr_trap_frame *tf);
void sys_fork (struct intr_trap_frame *tf);
void sys_execve (struct intr_trap_frame *tf);
void sys_exit (struct intr_trap_frame *tf);
void sys_wait (struct intr_trap_frame *tf);

/* Custom syscalls */
void sys_set_fg_proc (struct intr_trap_frame *tf);

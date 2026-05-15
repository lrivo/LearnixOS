#pragma once
#include <learnix/arch/interrupts.h>
#include <learnix/types.h>

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_GETPID 39
#define SYS_FORK 57
#define SYS_EXECVE 59
#define SYS_EXIT 60

/* This C function gets called by the syscall_entry assembly
 * routine for each HW architecture. */
void syscall_dispatcher (struct intr_trap_frame *tf);

/* Actual syscalls */
void sys_read (struct intr_trap_frame *tf);
void sys_write (struct intr_trap_frame *tf);
void sys_getpid (struct intr_trap_frame *tf);
void sys_fork (struct intr_trap_frame *tf);
void sys_execve (struct intr_trap_frame *tf);
void sys_exit (struct intr_trap_frame *tf);

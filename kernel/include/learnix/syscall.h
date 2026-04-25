#pragma once
#include <learnix/arch/syscall.h>
#include <learnix/types.h>

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_EXIT 60

/* This C function gets called by the syscall_entry assembly
 * routine for each HW architecture. */
void syscall_dispatcher (struct sys_trap_frame *tf);

/* Actual syscalls */
void sys_write (struct sys_trap_frame *tf);
void sys_exit (struct sys_trap_frame *tf);

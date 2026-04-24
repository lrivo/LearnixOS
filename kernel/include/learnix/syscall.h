#pragma once
#include <learnix/arch/syscall.h>
#include <learnix/types.h>

#define SYS_READ 0
#define SYS_WRITE 1

/* This C function gets called by the syscall_entry assembly
 * routine for each HW architecture. */
void syscall_dispatcher (struct sys_trap_frame *tf);

/* Actual syscalls */
void sys_write (struct sys_trap_frame *tf);

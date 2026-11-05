#pragma once
#include <learnix/arch/interrupts.h>
#include <learnix/types.h>
#include <uapi/learnix/syscall.h>
#include <uapi/learnix/errno.h>
#include <uapi/learnix/mman.h>

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

#pragma once
#include <learnix/types.h>

#define SYS_READ 0
#define SYS_WRITE 1

/* This C function gets called by the syscall_entry assembly
 * routine for each HW architecture.
 * It accepts and returns the widest type on the target arch. */
ssize_t syscall_dispatcher (size_t num, size_t arg0, size_t arg1, size_t arg2,
                            size_t arg3, size_t arg4, size_t arg5);

/* Actual syscalls */
ssize_t sys_write (uint32_t fd, const char *buf, size_t count);

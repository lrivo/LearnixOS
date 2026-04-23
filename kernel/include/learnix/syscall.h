#pragma once
#include <learnix/types.h>

/* This C function gets called by the syscall_entry assembly
 * routine for each HW architecture.
 * It accepts and returns the widest type on the target arch. */
ssize_t syscall_dispatcher (size_t num, size_t arg0, size_t arg1, size_t arg2,
                            size_t arg3, size_t arg4, size_t arg5);

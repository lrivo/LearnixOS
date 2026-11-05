#pragma once
#include <sys/types.h>
#include <learnix/syscall.h>

// NASM macro (liblearnix.asm) that performs a syscall
extern size_t _do_syscall(size_t number, size_t arg0, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5);

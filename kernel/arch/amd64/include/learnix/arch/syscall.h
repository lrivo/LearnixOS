#pragma once
#include <stdint.h>

/* x86_64 syscall trapframe */
struct sys_trap_frame
{
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rbp;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rbx;
  uint64_t rax;
};

/* macroes to extract the syscall arguments */
#define NUM(tf) ((tf)->rax)
#define ARG0(tf) ((tf)->rdi)
#define ARG1(tf) ((tf)->rsi)
#define ARG2(tf) ((tf)->rdx)
#define ARG3(tf) ((tf)->r10)
#define ARG4(tf) ((tf)->r8)
#define ARG5(tf) ((tf)->r9)

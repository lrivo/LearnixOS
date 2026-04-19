#pragma once
#include "types.h"

/* This defines the expected kernel stack frame when
 * we are handling an interrupt. */
struct intr_trap_frame
{
  // software-saved registers (isr_stubs.asm)
  uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi;
  uint64_t r8, r9, r10, r11, r12, r13, r14, r15;

  uint64_t vector_num; /* IDT entry */
  uint64_t error;      /* some exceptions optionally push an error code */

  // those are needed for iretq
  uint64_t rip;    /* userspace instruction pointer */
  uint64_t cs;     /* userspace code segment */
  uint64_t rflags; /* userspace flags */
  uint64_t rsp;    /* userspace stack pointer */
  uint64_t ss;     /* userspace data segment */
} __attribute__ ((packed));

#pragma once
#include <learnix/process.h>
#include <learnix/types.h>

/* This contains CPU core-specific information, like
 * the currently running process and any architecture
 * can define additional stuff in arch_cpu */
struct cpu
{
  struct cpu *self;
  bool proc_need_resched;
  uint32_t id;
  struct process *proc;
};

struct cpu_info
{
  /* architecture specific data, put first for casting */
  void *arch_data;

  /* human readable full name */
  char vendor[16];
  char name[64];

  /* address space */
  uint8_t pa_bits_max; // max physical address bit size
  uint8_t va_bits_max; // max virtual address bit size

/* known hardware bugs (eg: Meltdown, Spectre) */
#define CPU_BUG_MELTDOWN (1 << 0)
#define CPU_BUG_SPECTRE (1 << 1)
  uint64_t bugs;
};

/* Early minimal CPU initialization. */
void arch_stage_1 (void);

/* Full CPU initialization for running processes
 * and (in the future) SMP. */
void arch_stage_2 (void);

struct cpu *arch_cpu_get (void);

/* Identify the running CPU (es: cpuid on x86). */
void arch_cpu_identify (struct cpu_info *c);

/* Halts the CPU forever */
void arch_cpu_hcf (void);

#pragma once
#include <learnix/arch/cpu.h>
#include <learnix/process.h>
#include <learnix/types.h>

/* This contains CPU core-specific information, like
 * the currently running process and any architecture
 * can define additional stuff in arch_cpu */
struct cpu
{
  struct cpu *self;       // 0
  bool proc_need_resched; // 8
  uint32_t id;
  struct process *proc;          // 16
  struct arch_cpu_data cpu_data; // 24   WARN: must remain 24 bytes into the struct
};

/* This contains general information on the physical CPU identified
 * using instructions like CPUID on amd64. */
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

  /* TODO known hardware bugs (eg: Meltdown, Spectre) */
  #define CPU_BUG_MELTDOWN (1 << 0)
  #define CPU_BUG_SPECTRE (1 << 1)
  uint64_t bugs;
};

/* Minimal CPU initialization in early kmain().
 * Should at least setup exception handlers. */
void arch_stage_1 (void);

/* Full CPU initialization for running processes
 * and (in the future) SMP. */
void arch_stage_2 (void);

/* Returns the struct cpu of calling core. */
struct cpu *arch_cpu_get (void);

/* Identify the running CPU (es: cpuid on x86). */
void arch_cpu_identify (struct cpu_info *c);

/* Halts the CPU forever */
void arch_cpu_hcf (void);

/*
 * This header defines an architecture independent cpu_info
 * struct that the generic code can use to obtain informations
 * about the running CPU.
 */
#pragma once
#include <learnix/types.h>

struct cpu_info
{
  /* architecture specific data, put first for casting */
  void *arch_data;

  /* human readable full name */
  char name[64];

  /* address space */
  uint8_t pa_bits_max; // max physical address bit size
  uint8_t va_bits_max; // max virtual address bit size

/* known hardware bugs (eg: Meltdown, Spectre) */
#define CPU_BUG_MELTDOWN (1 << 0)
#define CPU_BUG_SPECTRE (1 << 1)
  uint64_t bugs;
};

/* Fills the cpu_info struct using architecture specific
   instructions like cpuid on x86-64. */
void arch_cpu_identify (struct cpu_info *c);

#pragma once
#include <learnix/arch/types.h>

/* Read the CR4 control register. */
inline uint64_t
rcr4 (void)
{
  uint64_t val;
  asm volatile ("mov %%cr4,%0" : "=r"(val));
  return val;
}

/* Write the CR4 control register. */
inline void
wcr4 (uint64_t val)
{
  asm volatile ("mov %0,%%cr4" ::"r"(val) : "memory");
}

/* Read MSR (Model Specific Register). */
inline uint64_t
rdmsr (uint32_t msr)
{
  uint32_t low, high;
  asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((uint64_t)high << 32) | low;
}

/* Write MSR (Model Specific Register). */
inline void
wrmsr (uint32_t msr, uint64_t value)
{
  asm volatile ("wrmsr" ::"c"(msr), "a"(value & 0xFFFFFFFF), "d"(value >> 32));
}

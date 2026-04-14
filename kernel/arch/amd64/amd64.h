#pragma once
#include <learnix/arch/amd64/types.h>

/* Read MSR (Model Specific Register). */
inline uint64_t 
rdmsr(uint32_t msr)
{
  uint32_t low, high;
  asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((uint64_t)high << 32) | low;
}

/* Write MSR (Model Specific Register). */
inline void
wrmsr (uint32_t msr, uint64_t value)
{
  asm volatile ("wrmsr" :: "c"(msr), "a"(value & 0xFFFFFFFF), "d"(value >> 32));
}

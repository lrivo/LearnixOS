#pragma once
#include "types.h"

/* Reads the time-stamp counter (TSC) of the calling core. The lfence makes
 * the read ordered, so short measurements aren't skewed by the pipeline. */
static inline uint64_t
arch_rdtsc (void)
{
  uint32_t lo, hi;
  asm volatile ("lfence; rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

struct arch_cpu_data
{
  /* SYSCALL */
  uint64_t usr_rsp; /* scratch space for saving userland RSP */
};

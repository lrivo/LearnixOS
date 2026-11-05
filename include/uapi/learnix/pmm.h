#pragma once
#include <stdint.h>

/* Userspace-visible PMM statistics (syscall PMM_STATS_GET). */
struct pmm_op_stats
{
  uint64_t count;  /* calls recorded in the window      */
  uint64_t min;    /* min cycles over the recorded calls */
  uint64_t mean;   /* average cycles over the recorded calls */
  uint64_t median; /* median cycles over the recorded calls */
  uint64_t max;    /* max cycles over the recorded calls */
};

struct pmm_stats
{
  struct pmm_op_stats alloc;
  struct pmm_op_stats unref;
};

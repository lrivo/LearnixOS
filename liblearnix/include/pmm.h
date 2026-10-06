#pragma once
#include <sys/types.h>

// userspace copy of kernel struct pmm_stats (kernel/include/learnix/mm/pmm_stats.h)
// layout must match exactly: the kernel memcpy's its own struct to this pointer
struct pmm_op_stats {
  uint64_t count;  // calls recorded in the window
  uint64_t min;    // min cycles over the recorded calls
  uint64_t mean;   // average cycles over the recorded calls
  uint64_t median; // median cycles over the recorded calls
  uint64_t max;    // max cycles over the recorded calls
};

struct pmm_stats {
  struct pmm_op_stats alloc;
  struct pmm_op_stats unref;
};

int pmm_stats_start(void);
int pmm_stats_get(struct pmm_stats *stats);

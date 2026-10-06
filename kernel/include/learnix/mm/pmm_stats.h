/*
 *  Physical Memory Manager statistics.
 */
#pragma once
#include <learnix/arch/cpu.h>
#include <learnix/types.h>

/* Maximum number of per-call cycle samples kept for each operation. */
#define PMM_STATS_MAX 4096

/* Userspace-visible results, mirrored in liblearnix/include/pmm.h. */
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

/* Set (1) while a measurement window is open. */
extern volatile int pmm_stats_active;

/* Opens a new measurement window, resetting the collected data. */
void pmm_stats_start (void);

/* Closes the window and fills out with the results. */
void pmm_stats_stop (struct pmm_stats *out);

/* Records a single call sample. */
void pmm_stats_record_alloc (uint64_t cycles);
void pmm_stats_record_unref (uint64_t cycles);

/* Backend hooks: the TSC is read only while a window is open. */
static inline uint64_t
pmm_stats_begin (void)
{
  return pmm_stats_active ? arch_rdtsc () : 0;
}

static inline void
pmm_stats_end_alloc (uint64_t tsc)
{
  if (pmm_stats_active)
    pmm_stats_record_alloc (arch_rdtsc () - tsc);
}

static inline void
pmm_stats_end_unref (uint64_t tsc)
{
  if (pmm_stats_active)
    pmm_stats_record_unref (arch_rdtsc () - tsc);
}

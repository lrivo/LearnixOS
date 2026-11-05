/*
 *  Physical Memory Manager statistics.
 */
#pragma once
#include <learnix/arch/cpu.h>
#include <learnix/types.h>
#include <uapi/learnix/pmm.h>

/* Maximum number of per-call cycle samples kept for each operation. */
#define PMM_STATS_MAX 4096

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

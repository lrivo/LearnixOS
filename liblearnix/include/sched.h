#pragma once
#include <sys/types.h>

// userspace copy of kernel struct sched_stats (kernel/include/learnix/sched_types.h)
// layout must match exactly: the kernel memcpy's its PCB field to this pointer
struct sched_stats {
  uint64_t ticks_running;    // tot ticks in RUNNING state
  uint64_t creation_tick;    // tick when this process was created      (fork)
  uint64_t first_sched_tick; // fist tick this was scheduled
  uint64_t tot_rescheds;     // how many times has this process been rescheduled
  uint64_t tot_wakeups;      // how many times has this process waken up
};

int sched_get_stats(struct sched_stats *stats);

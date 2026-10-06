#pragma once
#include <learnix/types.h>

// per-process schedulnig statistics, in the PCB
struct sched_stats {
  uint64_t ticks_running;    // tot ticks in RUNNING state
  uint64_t creation_tick;    // tick when this process was created      (fork)
  uint64_t first_sched_tick; // fist tick this was scheduled
  uint64_t tot_rescheds;     // how many times has this process been rescheduled
  uint64_t tot_wakeups;      // how many times has this process waken up
};

struct wait_queue {
  struct process *proc;
  struct wait_queue *next;
};
#pragma once
#include <learnix/process.h>

/* Scheduling entrypoint, called before returning from an interrupt
 * if needed. */
void schedule (void);

// === Sched Algorithms Interface === //

/* Initialization of needed data structure. */
void sched_init (void);

/* What this scheduling policy needs to do at every tick
 * (aka timer interrupt) */
void sched_tick (void);

/* Find the next process to run, policy-specific. */
struct process *sched_pick_next (struct process *curr);

/* Add a process to this scheduler's runqueue */
void sched_enqueue (struct process *p);

/* Remove a process from this scheduler's runqueue */
void sched_dequeue (struct process *p);

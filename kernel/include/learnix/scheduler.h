#pragma once
#include <learnix/process.h>

struct wait_queue
{
  struct process *proc;
  struct wait_queue *next;
};

/* Scheduling entrypoint, called before returning from an interrupt
 * if needed. */
void schedule (void);

/* Puts the current process in the given waitqueue. */
void sleep_on (struct wait_queue **wq);

/* Awakes the first process of the given waitqueue. */
void wake_up (struct wait_queue **wq);

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


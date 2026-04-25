/*
 * Round Robin scheduling algorithm.
 *
 * The runqueue is a circular doubly-linked list so that
 * insertion and deletion are both O(1).
 */
#pragma once
#include <learnix/types.h>

/* sched_data field for Round Robin. */
struct rr_node
{
  /* sched_tick() left before preemption. */
  uint64_t ticks_left;

  /* doubly-linked list pointers. */
  struct process *prev;
  struct process *next;
};

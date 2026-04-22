/*
 * Round Robin scheduling algorithm.
 *
 * The runqueue is a circular doubly-linked list so that
 * insertion and deletion are both O(1).
 */
#include "learnix/cpu.h"
#include "learnix/lib/kprintf.h"
#include "learnix/process.h"
#include <learnix/lib/kpanic.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/scheduler.h>
#include <stddef.h>

struct rr_node
{
  struct process *prev;
  struct process *next;
  uint64_t ticks_left;
};

static struct process *sentinel;

/* We want to "pretend" that the kernel idle process (PID = 0)
 * was already running and the next process is the init (PID = 1)
 * so that we switch between them immediately. */
void
sched_init ()
{
  // we use the kernel idle process as the sentinel node
  sentinel = proc_by_pid (0);
  sentinel->sched_data = kmalloc (sizeof (struct rr_node));
  ((struct rr_node *)sentinel->sched_data)->prev = sentinel;
  ((struct rr_node *)sentinel->sched_data)->next = sentinel;

  // insert the user init process
  sched_insert_proc (proc_by_pid (1));

  // force the initial CPU state
  // to fake that PID 0 was running
  arch_cpu_get ()->proc_need_resched = true;
  arch_cpu_get ()->proc = sentinel;
}

/* Pick the next runnable process */
struct process *
sched_pick_next (struct process *curr)
{
  return ((struct rr_node *)curr->sched_data)->next;
}

void
sched_tick (void)
{
  kprintf ("sched_tick: PID %u is running\n", arch_cpu_get ()->proc->pid);
  arch_cpu_get ()->proc_need_resched = true;
}

/* Linked-List tail insertion */
void
sched_insert_proc (struct process *p)
{
  // p must not have been alread inserted
  kassert (p != NULL && p != sentinel && p->sched_data == NULL);

  struct rr_node *new = kmalloc (sizeof (struct rr_node));
  struct rr_node *sent = (struct rr_node *)sentinel->sched_data;

  new->next = sentinel;
  new->prev = sent->prev; // current tail
  ((struct rr_node *)new->prev->sched_data)->next = p;
  sent->prev = p;

  p->sched_data = (void *)new;
}

/* Linked-List removal */
void
sched_remove_proc (struct process *p)
{
  // p must've been already inserted
  kassert (p != NULL && p != sentinel && p->sched_data != NULL);

  struct rr_node *curr = (struct rr_node *)p->sched_data;

  ((struct rr_node *)curr->prev->sched_data)->next = curr->next;
  ((struct rr_node *)curr->next->sched_data)->prev = curr->prev;

  kfree (curr);
  p->sched_data = NULL;
}

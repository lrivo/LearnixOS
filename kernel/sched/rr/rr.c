#include "rr.h"
#include "learnix/process.h"
#include <learnix/cpu.h>
#include <learnix/lib/kpanic.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/scheduler.h>

static struct process *sentinel;

void
sched_init ()
{
  // we use the kernel idle process as the sentinel node
  sentinel = proc_by_pid (0);
  sentinel->sched_data = kmalloc (sizeof (struct rr_node));
  ((struct rr_node *)sentinel->sched_data)->prev = sentinel;
  ((struct rr_node *)sentinel->sched_data)->next = sentinel;

  /* pretend the kernel idle process was running and immediately
   * needs to be rescheduled */
  arch_cpu_get ()->proc = sentinel;
  arch_cpu_get ()->proc_need_resched = true;
}

struct process *
sched_pick_next (struct process *curr)
{
  struct rr_node *r = (struct rr_node *)curr->sched_data;
  while (r->next->state != READY)
    r = (struct rr_node *)r->next->sched_data;

  return r->next;
}

void
sched_tick (void)
{
  arch_cpu_get ()->proc_need_resched = true;
}

void
sched_enqueue (struct process *p)
{
  // p must not be in the runqueue
  kassert (p != NULL && p != sentinel && p->sched_data == NULL);

  struct rr_node *new = kmalloc (sizeof (struct rr_node));
  struct rr_node *sent = (struct rr_node *)sentinel->sched_data;

  new->next = sentinel;
  new->prev = sent->prev; // current tail
  ((struct rr_node *)new->prev->sched_data)->next = p;
  sent->prev = p;

  p->sched_data = (void *)new;
}

void
sched_dequeue (struct process *p)
{
  // p must be in the runqueue
  kassert (p != NULL && p != sentinel && p->sched_data != NULL);

  struct rr_node *curr = (struct rr_node *)p->sched_data;

  ((struct rr_node *)curr->prev->sched_data)->next = curr->next;
  ((struct rr_node *)curr->next->sched_data)->prev = curr->prev;

  kfree (curr);
  p->sched_data = NULL;
}

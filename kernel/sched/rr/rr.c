#include "learnix/process.h"
#include <learnix/cpu.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/kpanic.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/scheduler.h>

static struct process *sentinel;

void
sched_init ()
{
  // we use the kernel idle process as the sentinel node
  sentinel = proc_by_pid(0);
  sentinel->prev = sentinel->next = sentinel;

  /* pretend the kernel idle process was running and immediately
   * needs to be rescheduled */
  arch_cpu_get()->proc = sentinel;
  arch_cpu_get()->proc_need_resched = true;

  kprintf("[INFO] round robin scheduler ready\n");
}

struct process *
sched_pick_next (struct process *curr)
{
  struct process *p = curr;
  while (p->next->state != READY)
  {
    p = p->next;

    /* if we re-encounter this process again we've visited
     * the entire runqueue and found no READY process. */
    if (p == curr)
      return sentinel;  // so we schedule the kernel idle
  }
  return p->next;
}

void
sched_tick (void)
{
  arch_cpu_get()->proc_need_resched = true;
}

void
sched_enqueue (struct process *p)
{
  kassert (p != NULL && p != sentinel);
  
  p->next = sentinel;
  p->prev = sentinel->prev;
  p->prev->next = p;
  sentinel->prev = p;
}

void
sched_dequeue (struct process *p)
{
  if (!p || p == sentinel) return;
  
  p->prev->next = p->next;
  p->next->prev = p->prev;
}

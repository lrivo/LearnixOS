#include "learnix/process.h"
#include <learnix/cpu.h>
#include <learnix/lib/string.h>
#include <learnix/lib/kprintf.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/scheduler.h>
#include <learnix/syscall.h>
#define RR_QUANTUM 10

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

  kprintf("[INFO] sched_rr ready\n");
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
  // 1. update the counters
  struct process *proc = arch_cpu_get()->proc;
  proc->sched_stats.ticks_running++;
  ticks++;

  kprintfdbg("[sched_tick] PID %d ticks %lu run_ticks: %lu left: %lu\n",
    (int)proc->pid, ticks, proc->sched_stats.ticks_running, proc->ticks_left);

  // with the idle process we can try to reschedule immediately
  if (proc == sentinel) {
    arch_cpu_get()->proc_need_resched = true;
  } else {
    if (--proc->ticks_left == 0) {
      proc->ticks_left = RR_QUANTUM;
      arch_cpu_get()->proc_need_resched = true;
      kprintfdbg("[sched_tick] PID %d exausted his quantum\n", (int)proc->pid);
    }
  }
}

void
sched_enqueue (struct process *p)
{
  if (!p || p == sentinel) return;

  // initialze the quantum slice
  p->ticks_left = RR_QUANTUM;

  // insert in the runqueue
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

int
sched_set_prio (struct process *p, uint64_t prio)
{
  (void)p;
  (void)prio;
  return -ENOTSUP;
}

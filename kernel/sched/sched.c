#include <learnix/cpu.h>
#include <learnix/lib/kprintf.h>
#include <learnix/scheduler.h>

void
schedule ()
{
  // try to find the next process to run
  struct process *curr = arch_cpu_get ()->proc;
  struct process *next = sched_pick_next (curr);
  if (next == NULL || curr == next)
    return; // nothing to schedule

  // update processes state
  if (curr->state == RUNNING)
  {
    curr->state = READY;
  }
  next->state = RUNNING;
  arch_cpu_get ()->proc = next;

  // context switch form curr to next
  arch_context_switch (curr, next);
}

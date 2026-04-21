#include <learnix/lib/kprintf.h>
#include <learnix/scheduler.h>

// FIXME: move this to a per-CPU struct
static struct process *curr;

void
schedule ()
{
  // try to find the next process to run
  struct process *next = sched_pick_next (curr);
  if (next == NULL || curr == next)
    return; // nothing to schedule

  // update processes state
  curr->state = READY;
  next->state = RUNNING;

  curr = next;

  // TEST:
  kprintf ("Context switch from %u => %u\n", curr->pid, next->pid);

  // context switch form curr to next
  arch_context_switch (curr, next);
}

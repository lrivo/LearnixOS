#include <learnix/cpu.h>
#include <learnix/lib/kprintf.h>
#include <learnix/scheduler.h>

void
schedule()
{
  // try to find the next process to run
  struct process *curr = arch_cpu_get()->proc;
  struct process *next = sched_pick_next(curr);
  if (next == NULL || curr == next)
    return; // nothing to schedule

  // update processes state
  if (curr->state == RUNNING)
    curr->state = READY;
  next->state = RUNNING;
  arch_cpu_get ()->proc = next;

  // context switch form curr to next
  arch_context_switch (curr, next);
}

/* Takes inspiration from Linux 0.0.1, the wait_queue node
   is a local variable on the process' kernel stack */
void
sleep_on(struct wait_queue **wq)
{
  struct process *curr = arch_cpu_get()->proc;
  struct wait_queue w = { curr, *wq };
  
  // mark the current process as waiting
  curr->state = WAITING;
 
  // linked list head insertion 
  *wq = &w; 

  // yield the CPU
  schedule();
 
  /* we'll wake up here after a wake_up() call as a RUNNING process.
     note that, since we are using the kernel stack, the w variable
     will clean itself when this function returns! */
}

void
wake_up(struct wait_queue **wq)
{
  // make sure pointers are valid
  if (wq && *wq)
  {
    // wake all the processes in the list
    struct wait_queue *tmp = *wq;
    while (tmp)
    {
      tmp->proc->state = READY;
      tmp = tmp->next;
    }
  }
  
  // just in case we clear the head
  *wq = NULL;
}

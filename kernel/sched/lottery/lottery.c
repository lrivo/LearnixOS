#include <learnix/cpu.h>
#include <learnix/scheduler.h>
#include <learnix/lib/rand.h>
#include <learnix/lib/kpanic.h>
#include <learnix/lib/kprintf.h>

#define INIT_TICKETS 100

static size_t tot_tickets = 0;
static struct process *runqueue;

/* Unlinke round robin, where the kernel idle is the sentinel node,
   we don't even need to include him in the runqueue here. */
void
sched_init()
{
  /* pretend the kernel idle was running */
  arch_cpu_get()->proc = proc_by_pid(0);
  arch_cpu_get()->proc_need_resched = true;

  kprintf("[INFO] lottery scheduler ready\n");
}

void
sched_tick()
{
  ticks++;
  arch_cpu_get()->proc_need_resched = true;
  arch_cpu_get()->proc->sched_stats.ticks_running++;
}

/* This runs the lottery */
struct process*
sched_pick_next (struct process *curr)
{
  // the lottery does not care about current process
  (void)curr;
  
  /* if the runqueue is empty we can only schedule the
     kernel idle process */
  if (tot_tickets == 0)
    return proc_by_pid(0);

  /* generate a random number between 0 and tot_tickets */
  size_t winner, cnt = 0;
  rand_bytes(&winner, sizeof(winner));
  winner = winner % tot_tickets;
  
  /* traverse the runqueue from the start. */
  struct process *current = runqueue;
  while (current != NULL)
  {
    cnt += current->priority;
    if (cnt > winner)
      break;  // current is the winner
    current = current->next;
  }
  
  kprintfdbg("[lottery] drawn PID %d\n", current->pid);

  return current;
}

void
sched_enqueue(struct process *p)
{
  kassert(p && p->priority == 0);

  if (tot_tickets > 0)
  {
    p->prev = NULL;
    p->next = runqueue;
    runqueue->prev = p;
  }
  else
  {
    p->prev = p->next = NULL;
  }
  runqueue = p;

  p->priority = INIT_TICKETS;
  tot_tickets += INIT_TICKETS;
}

void
sched_dequeue(struct process *p)
{
  // silently fail if p is not in the runqueue
  if (!p || p->priority == 0)
    return;

  if (p->prev != NULL)
    p->prev->next = p->next;
  else
    runqueue = p->next;

  if (p->next != NULL)
    p->next->prev = p->prev;

  tot_tickets -= p->priority;
  p->priority = 0;
  p->prev = p->next = NULL;
}

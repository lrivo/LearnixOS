#include <learnix/syscall.h>
#include <learnix/cpu.h>
#include <learnix/process.h>
#include <learnix/scheduler.h>

void
sys_wait (struct intr_trap_frame *tf)
{
  struct process *parent = arch_cpu_get()->proc, *child;
  while (1)
  {
    // search a ZOMBIE child of "parent" across all processes
    for (pid_t i = 0; i < PROCS_LEN; i++)
    {
      child = proc_by_pid(i);
      if (child == NULL || child->parent != parent || child->state != ZOMBIE)
        continue;
      
      /* NOTE: I guess it's more correct that exit() does
        this call since it will always run before this. 
        It became a bug while writing the lottery scheduler with
        the assumption that runqueue only has running processes that
        previously rr didn't have. */
      sched_dequeue(child);

      // we've found a ZOMBIE child we can free 
      proc_destroy(child);
      // and return his pid to the parent
      RET(tf) = i;
      // we can exit the wile finally
      return;
    }
    
    // BUG: if parent has no child this sleeps forever
    // no ZOMBIE child found, sleep untill one exits
    sleep_on(&parent->child_wq);
  }
}

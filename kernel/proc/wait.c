#include "learnix/lib/kprintf.h"
#include <learnix/syscall.h>
#include <learnix/cpu.h>
#include <learnix/process.h>
#include <learnix/scheduler.h>

void
sys_wait (struct intr_trap_frame *tf)
{
  // specifies a child to wait for, -1 means any child
  int target_pid = (int)ARG0(tf);

  struct process *parent = arch_cpu_get()->proc, *child;
  kprintfdbg("[sys_wait] PID %d\n", parent->pid);
  while (1)
  {
    // search a ZOMBIE child of "parent" across all processes
    for (pid_t i = 0; i < PROCS_LEN; i++)
    {
      child = proc_by_pid(i);

      // check if this is a ZOMBIE process child of parent
      if (child == NULL || child->parent != parent || child->state != ZOMBIE)
        continue;

      // only has effect if ARG0(tf) != -1
      if (target_pid != -1 && child->pid != (pid_t)target_pid)
          continue;

      kprintfdbg("[sys_wait] parent %d waken up on %d\n",
        (int)parent->pid, (int)child->pid);

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

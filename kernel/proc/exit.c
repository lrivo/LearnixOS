#include <learnix/cpu.h>
#include <learnix/syscall.h>
#include <learnix/scheduler.h>
#include <learnix/process.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/lib/kpanic.h>

/*
 * Exit terminates the calling process immediately.
 * 
 * The following must happen:
 * 1) any opened file descriptors are closed.
 * 2) any children of the process are inherited by PID 1.
 * 3) TODO: the parent of the process is sent a SIGCHLD signal
 * 4) the process becomes a ZOMBIE, and waits for his parent to reap him
 */
void
sys_exit (struct intr_trap_frame *tf)
{
  struct process *p = arch_cpu_get()->proc;

  // can't exit idle and init processes
  if (p->pid <= 1)
    kpanic("init process can't exit\n");

  // TODO: move this to a fd_close() helper
  // 1) close any opened file descriptor
  for (int i = 0; i < NFDS; i++) {
    if (!p->fds[i]) continue;
    if (--p->fds[i]->refcount == 0) {
      if (p->fds[i]->ops->close)
        p->fds[i]->ops->close(p->fds[i]);
      kfree(p->fds[i]);
      p->fds[i] = NULL;
    }
  }

  // 2) reparent all his childrens to PID 1
  for (pid_t i = 2; i < PROCS_LEN; i++) {
    struct process *child = proc_by_pid(i);
    if (child && child->parent == p) {
      // it's important to also wake_up init here
      child->parent = proc_by_pid(1);
      wake_up(&child->parent->child_wq);
    }
  }

  // 4) become a ZOMBIE and try to wake up the parent process
  p->state = ZOMBIE;
  wake_up(&p->parent->child_wq);
  
  // remove ourselves from the runqueue
  sched_dequeue(p);

  schedule();

  // will never return here
}

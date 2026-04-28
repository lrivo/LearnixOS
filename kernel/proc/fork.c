#include <learnix/cpu.h>
#include <learnix/lib/string.h>
#include <learnix/scheduler.h>
#include <learnix/syscall.h>

void
sys_fork (struct intr_trap_frame *tf)
{
  struct process *parent = arch_cpu_get ()->proc;
  struct process *child = proc_create ();

  // copy parent's userspace mappings into child->pgdir
  arch_copyuvm (parent, child);

  // TODO: correct but does duplicate work since memcpy will overwrite
  // initialize child's kernel stack
  arch_proc_init (child, 0, 0);

  // copy parent's trapframe into child's
  memcpy (child->tf, parent->tf, sizeof (struct intr_trap_frame));

  // parent's return value is child's PID
  RET (tf) = child->pid;

  // child's return value is 0
  RET (child->tf) = 0;

  // insert child in the runqueue
  sched_enqueue (child);
}

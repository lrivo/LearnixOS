#include "learnix/mm/vmm.h"
#include <learnix/cpu.h>
#include <learnix/lib/string.h>
#include <learnix/scheduler.h>
#include <learnix/syscall.h>

void
sys_fork(struct intr_trap_frame *tf)
{
  struct process *parent = arch_cpu_get()->proc;
  struct process *child = proc_create();

  child->parent = parent;

  // copy parent's userspace mappings into the child
  uvm_copy(child->pgtable, parent->pgtable);

  // initialize child->tf, the memcpy below will overwrite the
  // trapframe this function sets up 
  arch_proc_init(child, 0, 0);

  // copy parent's trapframe into child's
  memcpy(child->tf, parent->tf, sizeof (struct intr_trap_frame));
  
  // copy parent's file descriptor table
  for (int i = 0; i < NFDS; i++) {
    if (!parent->fds[i]) continue;

    child->fds[i] = parent->fds[i];
    child->fds[i]->refcount++;
  }

  // insert child in the runqueue
  sched_enqueue(child);
  
  // set the return values
  RET(tf) = child->pid;
  RET(child->tf) = 0;
}

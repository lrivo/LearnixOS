#include <learnix/arch/memlayout.h>
#include <learnix/lib/string.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/process.h>

// pid 0 is the fallback process (that is run when no other process exist)
static struct process *procs[PROCS_LEN] = { 0 };

/* returns the first free PID. */
static inline pid_t get_next_pid()
{
  for (pid_t i = 1; i < PROCS_LEN; i++)
    if (procs[i] == NULL) return i;  

  return -1;
}

void
proc_init (void)
{
  /* This idle process will just keep spinning in kernel mode,
     and it will be run if no other runnable process exists */
  struct process *idle = kzalloc (sizeof (struct process));
  idle->pid = 0;
  idle->state = RUNNING;
  idle->pgtable = vmm_get_kern_pgtable ();
  idle->kstack = P2V(pmm_alloc (PMM_NONE));

  procs[0] = idle;
}

struct process *
proc_by_pid (pid_t pid)
{
  return pid < PROCS_LEN ? procs[pid] : NULL;
}

struct process *
proc_create (void)
{
  struct process *p = kzalloc (sizeof (struct process));

  // give the process a pid
  p->pid = get_next_pid();
  p->state = READY;

  // give the process a fresh kernel stack
  p->kstack = P2V (pmm_alloc (PMM_ZERO));

  /* All process must inherit the kernel's page
   * table, otherwise we would have to switch it
   * at every interrupt/exception/syscall.
   * We memcpy the kernel's page table root in
   * another physical page.
   * NOTE: if the kernel where to add a new PML4
   * mapping (x86_64) processes created before it
   * won't see it. */
  p->pgtable = uvm_alloc();

  procs[p->pid] = p;

  return p;
}

// NOTE: the caller has already removed p from the runqueue
void
proc_destroy(struct process *p)
{
  // free his page table
  uvm_destroy(p->pgtable);

  // free his kernel stack
  pmm_unref_pg(V2P(p->kstack));
  
  // release his pid
  procs[p->pid] = NULL;

  // free his PCB
  kfree(p);
}

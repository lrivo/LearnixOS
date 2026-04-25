#include <learnix/arch/memlayout.h>
#include <learnix/lib/string.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/process.h>

// pid 0 is the fallback process (that is run when no other process exist)
static struct process *procs[32] = { 0 };
static pid_t pid_cnt = 1;

void
proc_init (void)
{
  /* This idle process will just keep spinning in kernel mode,
     and it will be run if no other runnable process exists */
  struct process *idle = kzalloc (sizeof (struct process));
  idle->pid = 0;
  idle->state = RUNNING;
  idle->pgtable = (void *)vmm_get_kern_pgtable ();
  idle->kstack = (void *)P2V (pmm_alloc (PMM_NONE));

  procs[0] = idle;
}

struct process *
proc_by_pid (pid_t pid)
{
  return pid < 32 ? procs[pid] : NULL;
}

struct process *
proc_create (void)
{
  struct process *p = kzalloc (sizeof (struct process));

  // give the process a pid
  p->pid = pid_cnt++;
  p->state = READY;

  // give the process a fresh kernel stack
  p->kstack = (void *)P2V (pmm_alloc (PMM_ZERO));

  /* All process must inherit the kernel's page
   * table, otherwise we would have to switch it
   * at every interrupt/exception/syscall.
   * We memcpy the kernel's page table root in
   * another physical page.
   * NOTE: if the kernel where to add a new PML4
   * mapping (x86_64) processes created before it
   * won't see it. */
  p->pgtable = (void *)P2V (pmm_alloc (PMM_NONE));
  memcpy (p->pgtable, (void *)vmm_get_kern_pgtable (), PGSIZE);

  procs[p->pid] = p;

  return p;
}

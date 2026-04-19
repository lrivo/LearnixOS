#include <learnix/arch/memlayout.h>
#include <learnix/lib/string.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/process.h>

// pid 0 is the fallback process (that is run when no other process exist)
static pid_t pid_cnt = 1;

struct process *
proc_create (void)
{
  struct process *p = kzalloc (sizeof (struct process));

  // give the process a pid
  p->pid = pid_cnt++;
  p->state = EMBRYO;

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
  memcpy (p->pgtable, (void *)vmm_get_pgtable (), PGSIZE);

  return p;
}

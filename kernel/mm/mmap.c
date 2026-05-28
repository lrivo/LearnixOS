#include <learnix/syscall.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/lib/kpanic.h>
#include <learnix/arch/memlayout.h>

/* we use a singly-linked list for global tracking of shared
 * mappings across all processes. 
 * BUG: currently I AM NOT CLEARING METADATAs, meaning that
 * a shared region reamins in this list even if the processes
 * that used it have exited.
 * For now YOU MUST CALL munmap() in USERSPACE. */
struct shared_listnode
{
  vaddr_t start;
  paddr_t pa;
  size_t length;
  size_t refcount;
  struct shared_listnode *next;
};
static struct shared_listnode *head = NULL;

/* handles a shared mapping, updating head if needed */
static paddr_t
mmap_shared(vaddr_t start, size_t length)
{
    struct shared_listnode *curr;
  // case first shared mapping
  if (!head)
  {
    head = (struct shared_listnode*)kmalloc(sizeof(struct shared_listnode));
    head->start = start;
    head->length = length;
    head->refcount = 1;
    head->pa = pmm_alloc(PMM_ZERO);
    head->next = NULL;
    return head->pa;
  }
  // case non-first shared mapping
  else
  {
    // try to find a shared mapping
    curr = head;
    while (curr->next)
    {
      if (curr->start == start && curr->length >= length)
        goto existing;
      curr = curr->next;
    }
    
    // is the last curr suitable?
    if (curr->start == start && curr->length >= length)
      goto existing;
    
    // if not this is a new shared mapping and we must
    // add him to the list
    struct shared_listnode *new = kmalloc(sizeof(struct shared_listnode));
    new->start = start;
    new->length = length;
    new->pa = pmm_alloc(PMM_ZERO);
    new->next = NULL;
    curr->next = new;
    return new->pa;
  }

existing:
  curr->refcount++;
  pmm_ref_pg(curr->pa);
  return curr->pa;
}

/*
 * mmap() creates a new mapping in the virtual address space of
 * the calling process.
 * size_t length: how many bytes to map
 *
 * void addr: 
 * if NULL the kernel chooses an address starting from MMAP_STAR defined in
 * the architecture's memlayout.h.
 * NOTE: if not NULL the kernel should trust the user but I am SKIPPING this
 * case for simplicity currently.
 * 
 * int prot
 * desired memory protections.
 * PROT_NONE
 * PROT_READ
 * PROT_WRITE
 * PROT_EXEC
 *
 * int flags
 * determines whether updates to the mapping are visible to other processes
 * mapping the same region.
 * MAP_PRIVATE: the calling process gets a dedicated physical frame only for himself
 * MAP_SHARED:  all processes mapping this range shares the same underlying
 *              physical frames (IPC)
 *
 * NOTE: currently does not support file mappings as Learnix does not
 * even have files itself :-)
 */
void
sys_mmap(struct intr_trap_frame *tf)
{
  // extract and validate syscall arguments
  vaddr_t addr = ARG0(tf);
  //size_t length = ARG1(tf);
  int prot = (int)ARG2(tf);
  int flags = (int)ARG3(tf);
  
  // choose a suitable address (for now we trust addr)
  vaddr_t va = PGROUNDDOWN(addr);

  // translate prot to vmm.h own flags
  int vmm_flags = VMM_FLAG_USER;
  if (prot & PROT_WRITE)
    vmm_flags |= VMM_FLAG_WRITE;
  if (prot & PROT_EXEC)
    vmm_flags |= VMM_FLAG_EXEC;
  
  // handle sharing
  paddr_t pf;
  if (flags == MAP_PRIVATE)
    // just request a fresh physical frame
    pf = pmm_alloc(PMM_ZERO);
  if (flags == MAP_SHARED)
    pf = mmap_shared(va, 4096);

  if (vmm_map(vmm_get_pgtable(), va, pf, vmm_flags) < 0)
  {
    // bad
    pmm_unref_pg(pf);
    RET(tf) = -1;
  }
  else 
    RET(tf) = va;
}

/*
 * This is a trust-user implementation because it doesn't validate
 * that the requested address was actually given with mmap() at all.
 *
 * NOTE: a proper implementation would require a global tracking of all
 * mmap()ed regions both shared and private but I don't have time for
 * it now.
 */
void
sys_munmap(struct intr_trap_frame *tf)
{
  vaddr_t addr = PGROUNDDOWN(ARG0(tf));
  size_t length = ARG1(tf);

  // SECURITY: a process can't unmap a kernel address
  if (!IS_USRADDR(addr))
    goto bad;
  
  /* try to unmap addr from the caller, vmm_unmap() returns
     -1 if it was not mapped in the fist place. */
  if (vmm_unmap(vmm_get_pgtable(), addr) < 0)
    goto bad;

  // if it was a shared mapping we must remove him
  struct shared_listnode *curr = head, *prev = NULL;
  while (curr)
  {
    if (curr->start == addr && curr->length == length)
    {
      if (--curr->refcount == 0)
      {
        // remove curr from the list
        if (prev)
          prev->next = curr->next;
        else
          head = curr->next;
        
        // and free his memory
        kfree(curr);
      }
      break;
    }
    prev = curr;
    curr = curr->next;
  }
  
  // success
  RET(tf) = 0;
  return;

bad:
  RET(tf) = -1;
}

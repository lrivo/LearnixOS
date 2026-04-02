#include "paging.h"
#include <learnix/lib/kprintf.h>
#include <learnix/mm/memlayout.h>
#include <learnix/mm/vmm.h>

/*
 * QUESTION: is it ok to leave this here instead of an arch independent
 * mm/vvm.c ??
 * Honestly, the mm/vmm.c is probably the better approach, but right now
 * I don't know how to properly design it so I'll leave it this way and
 * will refactor later if I have the time.
 *
 * those functions need pgdirwalk so I am putting them under arch/amd64
 * for this exact reason.
 * the rest of the kernel arch indipendent code will mostly use
 * kmall() btw.
 */

int
vmm_map (void *pgtable, vaddr_t va, physaddr_t pa, int flags)
{
  // make sure va is mapped, allocate missing pml3 and/or pml2
  pte_t *pte = pgdirwalk ((pml4_t *)pgtable, va, 1, NULL, NULL);

  if (pte)
  {
    // TODO parse the flags argument
    *pte = PGROUNDDOWN (pa) | PTE_PRESENT | PTE_WRITE;
    return 1;
  }
  else
  {
    return -1;
  }
}

physaddr_t
vmm_va_to_pa (void *pgtable, vaddr_t va)
{
  // walk the page table without allocating not present intermediate levels
  pte_t *pte = pgdirwalk ((pml4_t *)pgtable, va, 0, NULL, NULL);

  return pte ? (*pte & PTE_PA_MASK) + (va & 0x1FF) : 0;
}

inline vaddr_t
vmm_get_pgtable (void)
{
  vaddr_t val;
  asm volatile ("mov %%cr3,%0" : "=r"(val));
  return (vaddr_t)PA_TO_HHDM (val);
}

inline void
vmm_flush_single (vaddr_t va)
{
  asm volatile ("invlpg (%0)" ::"r"(va) : "memory");
}

inline void
vmm_flush_all (void)
{
  uint64_t cr3old;
  asm volatile ("mov %%cr3,%0" : "=r"(cr3old));
  asm volatile ("mov %0,%%cr3" : "=r"(cr3old));
}

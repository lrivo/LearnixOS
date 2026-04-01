#include "paging.h"
#include <learnix/mm/memlayout.h>
#include <learnix/mm/vmm.h>

physaddr_t
vmm_va_to_pa (void *pgtable, vaddr_t va) 
{
  // walk the page table without allocating not present intermediate levels 
  pte_t *pte = pgdirwalk((pml4_t*)pgtable, va, 0, NULL, NULL);
  
  if (pte)
  {
    int offset = va & 0x1FF;
    return (*pte & PTE_PA_MASK) + offset;
  }
  else
  {
    return 0;
  }
}

inline vaddr_t
vmm_get_pgtable (void)
{
  vaddr_t val;
  asm volatile ("mov %%cr3,%0" : "=r"(val));
  return (vaddr_t)PA_TO_HHDM(val); 
}

inline void
vmm_flush_single (vaddr_t va)
{
  asm volatile ("invlpg (%0)" :: "r"(va) : "memory"); 
}

inline void
vmm_flush_all (void)
{
  uint64_t cr3old;
  asm volatile ("mov %%cr3,%0" : "=r"(cr3old));
  asm volatile ("mov %0,%%cr3" : "=r"(cr3old));
}

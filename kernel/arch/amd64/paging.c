#include "paging.h"
#include <learnix/lib/kprintf.h>
#include <learnix/mm/memlayout.h>
#include <learnix/mm/pmm.h>

pte_t *
pgdirwalk (pml4_t *pml4, uintptr_t va, int flags, pml3_t **pml3out,
           pml2_t **pml2out)
{
  // HHDM pointer to the current level's page table
  uintptr_t *p = (uintptr_t *)pml4;

  // walk from pml4 to pml1 (pte)
  for (int i = 0, shift = 39; i < 3; i++, shift -= 9)
  {
    // extract the 0-511 index of the current level
    int idx = (va >> shift) & 0x1FF;

    // return the middle levels if requested
    if (pml3out && i == 1)
      *pml3out = (pml3_t *)&p[idx];
    if (pml2out && i == 2)
      *pml2out = (pml2_t *)&p[idx];

    // checks if the current level is not mapped
    if (!(p[idx] & PTE_PRESENT))
    {
      // if the caller requested we allocate this level by
      // requesting a zeroed physical frame to the PMM
      if (flags)
      {
        physaddr_t pf = pmm_alloc (PMM_ZERO);
        kprintf ("=> pgdirwalk allocated physical frame 0x%lx\n", pf);
        p[idx] = pf | PTE_WRITE | PTE_PRESENT; // TODO: adjout mapping flags
      }
      else
      {
        return NULL;
      }
    }

    // stop early if pml3 (1 GB)  or pml2 (2 MB) are huge pages
    // NOTE: using 5 level paging you also need to check for pml4
    if (i > 0 && p[idx] & PTE_HUGE)
      return (pte_t *)&p[idx];

    // make p point to the HHDM address of the next level
    p = (uintptr_t *)PA_TO_HHDM (p[idx] & PTE_PA_MASK);
  }

  // if we exit the loop the pte exists, return his HHDM virtual address
  return (pte_t *)&p[(va >> 12) & 0x1FF];
}

int
pml_unused (vaddr_t pml)
{
  uintptr_t *p = (uintptr_t *)PGROUNDDOWN (pml);
  for (int i = 0; i < 512; i++)
    if (p[i] & PTE_PRESENT)
      return 0;
  return 1;
}

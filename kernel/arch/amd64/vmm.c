#include "learnix/arch/types.h"
#include "paging.h"
#include <learnix/arch/memlayout.h>
#include <learnix/mm/pmm.h>
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

static int
pml_unused (vaddr_t pml)
{
  uintptr_t *p = (uintptr_t *)PGROUNDDOWN (pml);
  for (int i = 0; i < 512; i++)
    if (p[i] & PTE_PRESENT)
      return 0;
  return 1;
}

static pte_t *
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
        paddr_t pf = pmm_alloc (PMM_ZERO);
        p[idx] = pf | PTE_WRITE | PTE_PRESENT;
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
    p = (uintptr_t *)P2V (p[idx] & PTE_PA_MASK);
  }

  // if we exit the loop the pte exists, return his HHDM virtual address
  return (pte_t *)&p[(va >> 12) & 0x1FF];
}

int
vmm_map (void *pgtable, vaddr_t va, paddr_t pa, int flags)
{
  pml4_t *pml4 = (pml4_t *)pgtable;
  pml3_t *pml3;
  pml2_t *pml2;

  // make sure va is mapped, allocate missing pml3 and/or pml2
  pte_t *pte = pgdirwalk (pml4, va, 1, &pml3, &pml2);

  if (pte)
  {
    uint64_t pte_flags = PTE_PRESENT | PTE_WRITE;
    // make sure all levels are mapped as user
    if (flags & VMM_FLAG_USER)
    {
      pml4[PML4_IDX (va)] |= PTE_USER;
      *pml3 |= PTE_USER;
      *pml2 |= PTE_USER;
      pte_flags |= PTE_USER;
    }
    // map only the PTE as cache-disabled
    if (flags & VMM_FLAG_NOCACHE)
    {
      pte_flags |= PTE_PWT | PTE_PCD;
    }
    *pte = PGROUNDDOWN (pa) | pte_flags;
    vmm_flush_single (va);
    return 1;
  }
  else
  {
    return -1;
  }
}

int
vmm_unmap (void *pgtable, vaddr_t va)
{
  /* in this case we want pgdirwalk to return us pointers to
     the intermediate levels, in case we need to free them too. */
  pml4_t *pml4 = (pml4_t *)pgtable;
  pml3_t *pml3;
  pml2_t *pml2;

  // check if va is actually mapped in pgtable
  pte_t *pte = pgdirwalk (pml4, va, 0, &pml3, &pml2);
  if (!pte)
    return -1;

  // mark the pte not present and flush va from the TLB
  *pte &= ~PTE_PRESENT;
  vmm_flush_single (va);

  // try to free the physical frame of va
  pmm_unref_pg (*pte & PTE_PA_MASK);

  /* if the pte is unused (all entries ~PTE_PRESENT) we can free
     the underlying physical frame and mark it as not present
     in the pml2. */
  if (pml2 && pml_unused ((vaddr_t)pte))
  {
    // the physical address of pte is found in the pml2 entry
    paddr_t pa = *pml2 & PTE_PA_MASK;
    *pml2 = 0;
    pmm_unref_pg (pa);
  }

  /* if pml2 is unused we can free the underlying physical
    frame and mark it as not present in the pml3. */
  if (pml3 && pml_unused ((vaddr_t)pml2))
  {
    // the physical address of pml2 is found in the pml3 entry
    paddr_t pa = *pml3 & PTE_PA_MASK;
    *pml3 = 0;
    pmm_unref_pg (pa);
  }

  /* if pml3 is unused we can free the underlying physical
     frame and mark it as not present in the pml4. */
  if (pml3 && pml_unused ((vaddr_t)pml3))
  {
    // the physical address of pml3 is found in the pml4 entry
    paddr_t pa = pml4[PML4_IDX (va)] & PTE_PA_MASK;
    pml4[PML4_IDX (va)] = 0;
    pmm_unref_pg (pa);
  }

  return 1;
}

paddr_t
vmm_va_to_pa (void *pgtable, vaddr_t va)
{
  // walk the page table without allocating not present intermediate levels
  pte_t *pte = pgdirwalk ((pml4_t *)pgtable, va, 0, NULL, NULL);

  return pte ? (*pte & PTE_PA_MASK) + (va & 0xFFF) : 0;
}

inline vaddr_t
vmm_get_pgtable (void)
{
  vaddr_t val;
  asm volatile ("mov %%cr3,%0" : "=r"(val));
  return (vaddr_t)P2V (val & ~0xFFFULL);
}

inline void
vmm_swap_pgtable (void *new_pgtable)
{
  asm volatile ("mov %0,%%cr3" ::"r"(V2P (new_pgtable)));
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
  asm volatile ("mov %0,%%cr3" ::"r"(cr3old));
}

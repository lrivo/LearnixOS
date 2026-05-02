#include "paging.h"
#include <learnix/arch/memlayout.h>
#include <learnix/arch/vmm.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>

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
arch_pg_map (vaddr_t pgtable, vaddr_t va, paddr_t pa, int flags)
{
  pml4_t *pml4 = (pml4_t *)pgtable;
  pml3_t *pml3;
  pml2_t *pml2;

  // make sure va is mapped, allocate missing pml3 and/or pml2
  pte_t *pte = pgdirwalk (pml4, va, 1, &pml3, &pml2);

  // translate the generic flags to x86_64
  if (pte)
  {
    // by default we make the page present, readable and not executable
    uint64_t pte_flags = PTE_PRESENT | PTE_NX;
    
    // set the write bit if requested 
    if (flags & VMM_FLAG_WRITE)
      pte_flags |= PTE_WRITE;

    // clear the NX bit for executable pages
    if (flags & VMM_FLAG_EXEC)
      pte_flags &= ~PTE_NX;
    
    // map only the PTE as cache-disabled
    if (flags & VMM_FLAG_NOCACHE)
      pte_flags |= PTE_PWT | PTE_PCD;

    // SECURITY: assert va belongs to the userland split
    // make sure all levels are mapped as user
    if (flags & VMM_FLAG_USER)
    {
      pml4[PML4_IDX (va)] |= PTE_USER;
      *pml3 |= PTE_USER;
      *pml2 |= PTE_USER;
      pte_flags |= PTE_USER;
    }

    // update the pte and flush the TLB
    *pte = PGROUNDDOWN (pa) | pte_flags;
    arch_tlb_flush (va);
    return 0;
  }
  else
    return -1;
}

int
arch_pg_unmap (vaddr_t pgtable, vaddr_t va)
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
  arch_tlb_flush (va);

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

  return 0;
}

paddr_t
arch_va_to_pa (vaddr_t pgtable, vaddr_t va)
{
  // walk the page table without allocating not present intermediate levels
  pte_t *pte = pgdirwalk ((pml4_t *)pgtable, va, 0, NULL, NULL);

  return pte ? (*pte & PTE_PA_MASK) + (va & 0xFFF) : 0;
}

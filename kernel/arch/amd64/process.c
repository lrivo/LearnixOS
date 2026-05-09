/*
 *  x86_64 arch functions for processes.
 */
#include "gdt.h"
#include "learnix/mm/pmm.h"
#include "learnix/mm/vmm.h"
#include "paging.h"
#include <learnix/arch/memlayout.h>
#include <learnix/arch/types.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
#include <learnix/process.h>

/* This is an assembly trampoline function that should only
   be called the first time a process gets scheduled. */
extern void jump_usr_first_time (void);

/* This swaps the kernel stack from prev to next */
extern void _switch_to (struct arch_proc_context *prev,
                        struct arch_proc_context *next);

// TODO: this is only used by copyuvm() to correctly inherit the
// paging flags of the parent (especially the NX bit).
static int
pte_extract_flags(pte_t *pte)
{
  int flags = 0;

  if (*pte & PTE_WRITE)
    flags |= VMM_FLAG_WRITE;
  if (*pte & PTE_USER)
    flags |= VMM_FLAG_USER;
  if (*pte & PTE_GLOBAL)
    flags |= VMM_FLAG_GLOBAL;
  if (!(*pte & PTE_NX))
    flags |= VMM_FLAG_EXEC; 

  return flags;
}

void
arch_proc_init (struct process *p, vaddr_t user_ip, vaddr_t user_sp)
{
  /* initialize the p->tf pointer */
  p->tf = (struct intr_trap_frame *)((vaddr_t)p->kstack + KSTACK_SIZE
                                     - sizeof (struct intr_trap_frame));

  /* we set all registers to 0 on first run */
  memset ((void *)p->tf, 0x00, sizeof (struct intr_trap_frame));

  /* iretq frame setup */
  p->tf->rip = user_ip;
  p->tf->cs = GDT_UCODE | 3;
  p->tf->rflags = 0x202; // IF (allow ring3->ring0 intr) | BRKI
  p->tf->rsp = user_sp;
  p->tf->ss = GDT_UDATA | 3;

  /* put the jump_user_first_time trampoline
     before the trap frame. */
  uint64_t *sp = (uint64_t *)p->tf;

  // this is the return address for _switch_to
  *(--sp) = (uint64_t)jump_usr_first_time;
  *(--sp) = 0; // rbx
  *(--sp) = 0; // rsp
  *(--sp) = 0; // r12
  *(--sp) = 0; // r13
  *(--sp) = 0; // r14
  *(--sp) = 0; // r15

  p->ctx.rsp = (uint64_t)sp;
}

void
arch_proc_exec(struct process *p, vaddr_t user_ip, vaddr_t user_sp)
{
  /* initialize the p->tf pointer */
  p->tf = (struct intr_trap_frame *)((vaddr_t)p->kstack + KSTACK_SIZE
                                     - sizeof (struct intr_trap_frame));

  /* we set all registers to 0 on first run */
  memset ((void *)p->tf, 0x00, sizeof (struct intr_trap_frame));

  /* iretq frame setup */
  p->tf->rip = user_ip;
  p->tf->cs = GDT_UCODE | 3;
  p->tf->rflags = 0x202; // IF (allow ring3->ring0 intr) | BRKI
  p->tf->rsp = user_sp;
  p->tf->ss = GDT_UDATA | 3;

  // TODO: this could also go in kernel_exit after call schedule
  // but for now I'll put it here
  tss_set_rsp0(p->kstack + KSTACK_SIZE);
  asm volatile ("mov %0,%%cr3" ::"r"(V2P(p->pgtable)));
}

void
arch_context_switch (struct process *prev, struct process *next)
{
  // switch TSS.rsp0
  tss_set_rsp0 ((vaddr_t)next->kstack + KSTACK_SIZE);

  /* change address space at the last possible moment
     before context switching as this flushes the TLB. */
  asm volatile ("mov %0,%%cr3" ::"r"(V2P (next->pgtable)));

  // assembly stub that does the actual switch
  _switch_to (&prev->ctx, &next->ctx);

  /* we'll re-enter here the next time prev will be
     scheduled. */
}

void
arch_uvm_copy_or_destroy(vaddr_t dest, vaddr_t src, bool destroy)
{
  // walk parent's PML4 user split
  pml4_t *pml4 = (pml4_t *)src;
  for (size_t i = 0; i < 256; i++)
  {
    // present PML4 entry found, search the pml3
    if (pml4[i] & PTE_PRESENT)
    {
      pml3_t *pml3 = (pml3_t *)P2V (pml4[i] & PTE_PA_MASK);
      for (size_t j = 0; j < 512; j++)
      {
        if (pml3[j] & PTE_PRESENT)
        {
          pml2_t *pml2 = (pml2_t *)P2V (pml3[j] & PTE_PA_MASK);
          for (size_t k = 0; k < 512; k++)
          {
            // we found a PTE
            if (pml2[k] & PTE_PRESENT)
            {
              pte_t *pte = (pte_t *)P2V (pml2[k] & PTE_PA_MASK);
              for (size_t z = 0; z < 512; z++)
              {
                // we found an actual page
                if (pte[z] & PTE_PRESENT)
                {
                  // reconstruct his virtual address from i,j,k,z
                  vaddr_t va = VADDR_IDXS (i, j, k, z);

                  // if we need to destroy
                  if (destroy)
                  {
                    vmm_unmap(src, va);
                  }
                  else
                  {
                    // get his physical address
                    paddr_t pa_parent = pte[z] & PTE_PA_MASK;

                    // request a physical page for the child
                    paddr_t pa_child = pmm_alloc (PMM_NONE);
                    memcpy ((void *)P2V (pa_child), (void *)P2V (pa_parent),
                          PGSIZE);
                  
                    // map va with the same flags of the parent
                    vmm_map (dest, va, pa_child, pte_extract_flags(pte));
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

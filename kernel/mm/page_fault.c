/*
 * This file implements the arch-independent page fault handler, that operates
 * on top of VMAs.
 * The arch-specific handler will gather and translate informations and then
 * call this one to handle the fault.
 * VMAs shouldn't be known to the architecture specific code.
 */
#include "learnix/mm/mm_types.h"
#include <learnix/arch/memlayout.h>
#include <learnix/cpu.h>
#include <learnix/mm/vmm.h>
#include <learnix/mm/vma.h>
#include <learnix/mm/pmm.h>
#include <learnix/lib/string.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/kpanic.h>
#include <stdint.h>

/* Those are architecture indipendent page fault flags.
   Note that some are mutually exclusive. */
#define PF_PROT       (1ULL << 0)    // page protection violation
#define PF_WRITE      (1ULL << 1)    // write operation
#define PF_USER       (1ULL << 2)    // usermode
#define PF_INSTRFETCH (1ULL << 3)    // instruction fetch

// returns 1 if flags are compatible with vma permissions
static inline int
vma_access_allowed(struct vm_area *vma, uint64_t flags) {
    // a write caused the #PF
    if (flags & PF_WRITE) {
        if (!(vma->flags & VMA_WRITE)) return 0;
    } else {
        if (!(vma->flags & VMA_READ)) return 0;
    }

    // a non-executable region caused the #PF
    if (flags & PF_INSTRFETCH) {
        if (!(vma->flags & VMA_EXEC)) return 0;
    }

    // OK
    return 1;
}

// TODO: my paging code has no way to express non present and non readable
static inline size_t
vma_flags_to_vmm(struct vm_area *vma) {
    size_t flags = VMM_FLAG_USER;
    if (vma->flags & VMA_WRITE)
        flags |= VMM_FLAG_WRITE;
    if (vma->flags & VMA_EXEC)
        flags |= VMM_FLAG_EXEC;
    return flags;
}

int
mm_page_fault(vaddr_t fault, uint64_t flags) {
    struct process *proc = arch_cpu_get()->proc;

    // faults on kernel addresses are never recoverable for now
    if (IS_KVADDR(fault))
        goto kpanic;

    // does fault falls in a VMA?
    struct vm_area *vma = vma_search(proc->mm->vma_head, fault);
    if (!vma)
        goto sigsegv;

    if (flags & PF_PROT) {
        // CAUSE: protection fault
        if (!vma_access_allowed(vma, flags))
            goto sigsegv;
        // TODO: change HW flags
    } else {
        // CAUSE: non present page
        if (!vma_access_allowed(vma, flags))
            goto sigsegv;
        if (vma->flags & VMA_FILEBACKED) {
            // how many bytes into the vma
            uint64_t vma_off = (PGROUNDDOWN(fault) - vma->start);
            // how many bytes to actually copy, clamped to PGSIZE
            uint64_t n = vma->backing_size - vma_off;
            if (n > PGSIZE) n = PGSIZE;
            // request a physical frame
            paddr_t pa = pmm_alloc((n == PGSIZE) ? PMM_NONE : PMM_ZERO);
            // copy file contents into it
            memcpy((void*)P2V(pa), (void*)vma->backing + vma->backing_offset + vma_off, n);
            // virtually map it at fault
            vmm_map(proc->mm->pgtable, fault, pa, vma_flags_to_vmm(vma));
        } else {
            // anonymous VMA
            vmm_map(proc->mm->pgtable, fault, pmm_alloc(PMM_ZERO), vma_flags_to_vmm(vma));
        }
    }

    // fault handled correctly
    return 2;

sigsegv:
    // kill the offending process
    kprintf("SIGSEGV at 0x%p\n", fault);
    return 1;

kpanic:
    // unrecoverable, let caller kpanic
    return 0;
}

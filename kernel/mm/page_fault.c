/*
 * This file implements the arch-independent page fault handler, that operates
 * on top of VMAs.
 * The arch-specific handler will gather and translate informations and then
 * call this one to handle the fault.
 * VMAs shouldn't be known to the architecture specific code.
 */
#include <learnix/cpu.h>
#include <learnix/mm/vmm.h>
#include <learnix/mm/vma.h>
#include <learnix/mm/pmm.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/kpanic.h>

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

    // usermode caused the fault
    if (flags & PF_USER) {
        // does a vma with this address exists?
        struct vm_area *v = vma_search(proc->vma_head, fault);
        if (!v)
            goto sigsegv;
        
        // a vma exists, determine why we faulted 
        if (flags & PF_PROT) {
            // we faulted because of page table's permissions violation
            if (vma_access_allowed(v, flags)) {
                // TODO: need to change HW flags
            } else {
                goto sigsegv;
            }
        } else {
            // fault -> page not present
            if (vma_access_allowed(v, flags)) {
                if (v->flags & VMA_FILEBACKED) {
                    // FILE BACKED MAPPING TODO
                } else {
                    // DEMAND PAGING (virtually allocate a zeroed page frame)
                    vmm_map(proc->pgtable, fault, pmm_alloc(PMM_ZERO), vma_flags_to_vmm(v));
                }
            } else {
                goto sigsegv;
            }
        }
       
        // fault handled correctly
        return 2;
    } else { 
        // fault happened in kernel mode
        goto kpanic;
    }
    
sigsegv:
    // kill the offending process
    kprintf("SIGSEGV at 0x%p\n", fault);
    return 1;

kpanic:
    // unrecoverable, let caller kpanic 
    return 0;
}

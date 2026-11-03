#include "learnix/arch/interrupts.h"
#include "learnix/arch/types.h"
#include "learnix/mm/mm_types.h"
#include "learnix/mm/vmm.h"
#include <learnix/cpu.h>
#include <learnix/syscall.h>
#include <learnix/mm/vma.h>
#include <learnix/arch/memlayout.h>
#include <learnix/lib/kprintf.h>

static inline uint64_t
prot_to_vma_flags(int prot) {
    uint64_t flags = 0;
    if (prot & PROT_NONE)
        return flags;
    if (prot & PROT_READ)
        flags |= VMA_READ;
    if (prot & PROT_WRITE)
        flags |= VMA_WRITE;
    if (prot & PROT_EXEC)
        flags |= VMA_EXEC;
    return flags;
}

// REFACTOR: this will become a do_mmap() and the sys_mmap()
// will be moved in the syscall subsystem and just call do_mmap()
// after argument validation
void
sys_mmap(struct intr_trap_frame *tf) {
    struct process *curr = arch_cpu_get()->proc;
    size_t ret = 0;
    vaddr_t addr = (vaddr_t)ARG0(tf);
    size_t length = (size_t)ARG1(tf);
    int prot = (int)ARG2(tf);
    //int flags = (int)ARG3(tf);          // IGNORED, MAP_PRIVATE is assumed for now

    // POSIX: if len is 0 mmap shall fail
    if (length == 0) {
        ret = -EINVAL; goto bad;
    }

    // specific address passed
    if (addr != 0) {
        // must be in userspace
        if (!IS_USRADDR(addr)) {
            ret = -EINVAL; goto bad;
        }
        // must not be already mapped in a VMA
        if (vma_search(curr->mm->vma_head, addr)) {
            ret = -EEXIST; goto bad;
        }
    } else {
        // TODO: find a valid addess, skip for now
        ret = -ENOTSUP; goto bad;
    }

    // create a VMA for this mapping
    struct vm_area *new = vma_alloc(addr, addr + PGROUNDUP(length), prot_to_vma_flags(prot));
    if (vma_insert(&curr->mm->vma_head, new)) {
        RET(tf) = (size_t)addr;
    } else {
        RET(tf) = -EEXIST; goto bad;
    }
    return;

bad:
    RET(tf) = ret;
}

void
sys_munmap(struct intr_trap_frame *tf) {
    vaddr_t addr = (vaddr_t)ARG0(tf);
    size_t length = (size_t)ARG1(tf), ret = 0;

    if (addr == 0 || IS_KVADDR(addr)) {
        ret = -EINVAL; goto bad;
    }
    if (length == 0) {
        ret = -EINVAL; goto bad;
    }

    struct process *p = arch_cpu_get()->proc;
    struct vm_area *vma = vma_search(p->mm->vma_head, addr);
    if (!vma) {
        ret = -EINVAL; goto bad;
    }
    // FIXME: since I don´t have a way to split a vma in two for now
    // I'll just require to delete the entire mapping
    if (vma->end - vma->start != length) {
        ret = -EINVAL; goto bad;
    }
    // remove the VMA entry
    vma_remove(&p->mm->vma_head, vma);
    // finally, update the page tables
    vmm_unmap_range(p->mm->pgtable, vma->start, (vma->end - vma->start) / PGSIZE);

bad:
    RET(tf) = ret;
}

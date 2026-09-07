#include <learnix/cpu.h>
#include <learnix/syscall.h>
#include <learnix/mm/vma.h>
#include <learnix/arch/memlayout.h>
#include <learnix/lib/kprintf.h>

static inline void vma_print(struct vm_area *curr) {   
    while (curr) {
        kprintf("0x%p 0x%p - ", curr->start, curr->end);  
        if (curr->flags & VMA_READ) kprintf("R "); 
        if (curr->flags & VMA_WRITE) kprintf("W "); 
        if (curr->flags & VMA_EXEC) kprintf("X "); 
        kprintf("\n");
        curr = curr->next;
    }
} 

void
sys_mmap(struct intr_trap_frame *tf) {
    struct process *curr = arch_cpu_get()->proc;
    size_t ret = 0;
    vaddr_t addr = (vaddr_t)ARG0(tf);
    size_t length = (size_t)ARG1(tf);
    int prot = (int)ARG2(tf);
    int flags = (int)ARG3(tf);
    
    // specific address passed 
    if (addr != 0) {
        // must be in userspace 
        if (!IS_USRADDR(addr)) {
            ret = -EINVAL; goto bad;
        }

        // must not be already mapped in a VMA
        if (vma_search(curr->vma_head, addr)) {
            ret = -EEXIST; goto bad; 
        }
    } else {
        // TODO: find a valid addess, skip for now
        ret = -EINVAL; goto bad;
    }
    
    // convert prot into vma flags 
    size_t vma_flags = 0;
    if (prot & PROT_READ)
        vma_flags |= VMA_READ;
    if (prot & PROT_WRITE)
        vma_flags |= VMA_WRITE;
    if (prot & PROT_EXEC)
        vma_flags |= VMA_EXEC;

    // now we can insert the address
    struct vm_area *new = vma_alloc(addr, addr + 4096, vma_flags);
    if (vma_insert(&curr->vma_head, new)) {
        RET(tf) = (size_t)addr;
    } else {
        RET(tf) = -EEXIST; goto bad;
    }

    vma_print(curr->vma_head);
    return;

bad:
    RET(tf) = ret;
}

void
sys_munmap(struct intr_trap_frame *tf) {
    RET(tf) = -EINVAL;
}

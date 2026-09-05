#include <learnix/arch/memlayout.h>
#include <learnix/mm/vma.h>
#include <learnix/mm/kmalloc.h>

struct vm_area*
vma_alloc(vaddr_t start, vaddr_t end, uint64_t flags) {
    struct vm_area *vma = kmalloc(sizeof(struct vm_area));
    if (vma) {
        vma->start = PGROUNDDOWN(start);
        vma->end = PGROUNDUP(end);
        vma->flags = flags;
        vma->next = NULL;
    }
    return vma;
}

void
vma_free(struct vm_area *vma) {
    // TODO we should also remove it from the linked list
    // maybe insert a recursive pointer to its head (in parent PCB)
    if (vma) kfree(vma);
}

// TODO catch overlaps
int
vma_insert(struct vm_area **head, struct vm_area *vma) {
    if (!head || !vma)
        return -1;

    while (*head && vma->start > (*head)->end) {
        head = &((*head)->next);
    }
    vma->next = *head;
    *head = vma;
    return 0;
}

int
vma_remove(struct vm_area **head, struct vm_area *vma) {
    if (!head || !vma)
        return -1;
    
    // traverse the list untill we finish or we find vma 
    while (*head && *head != vma) {
        // exit early if the end of vma is less than start of current entry
        if ((*head)->start > vma->end)
            return -1;
        // otherwise, advance
        head = &((*head)->next);
    }

    if (!*head)
        return -1;
    
    /* this works before *head is pointing to prev->next. */
    *head = (*head)->next;
    return 0;
}

struct vm_area*
vma_search(struct vm_area *head, vaddr_t va) {
    struct vm_area *curr = head;
    while (curr) {
        if (va >= curr->start && va <= curr->end) break; 
        curr = curr->next;
    }
    return curr;
}

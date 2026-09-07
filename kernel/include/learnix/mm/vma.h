/*
 * vma.h (Virtual Memory Area) defines types and methods for managing virtual memory areas.
 * A virtual memory area is a contiguous block of virtual memory that is mapped with the same permissions,
 * or even backed by a file, in the address space of a process.
 * It allows for things like deman paging and CoW fork.
 */
#pragma once
#include <learnix/types.h>

/* Those are the values for vm_area->flags. */
#define VMA_READ       (1UL << 0)
#define VMA_WRITE      (1UL << 1)
#define VMA_EXEC       (1UL << 2)
#define VMA_FILEBACKED (1UL << 3)

/* This struct represents a virtual memory area. */
struct vm_area {
    // first page-aligned virtual address
    vaddr_t start;
    // first virtual address NOT in this vma (exclusive)
    vaddr_t end;
    // virtual memory protections for this region
    uint64_t flags;
    // next vma for the process
    struct vm_area *next;
};

/* --------------------------------------------------- */

/* Kmallocs a new struct vm_area and returns a pointer to it. */
struct vm_area *vma_alloc(vaddr_t start, vaddr_t end, uint64_t flags);

/* Frees vma. */
void vma_free(struct vm_area *vma);

/* Inserts vma into the sorted linked list. */
int vma_insert(struct vm_area **head, struct vm_area *vma);

/* Removes vma from the sorted linked list. */
int vma_remove(struct vm_area **head, struct vm_area *vma);

/* Tries to find a vma that contains va (vma->start <= vma <= vm->end) */ 
struct vm_area *vma_search(struct vm_area *head, vaddr_t va);

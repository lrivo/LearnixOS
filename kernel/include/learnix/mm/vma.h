/*
 * vma.h (Virtual Memory Area) declares the methods for managing virtual memory areas.
 * A virtual memory area is a contiguous block of virtual memory that is mapped with the same permissions,
 * or even backed by a file, in the address space of a process.
 * It allows for things like deman paging and CoW fork.
 * The types themselves (struct vm_area, VMA_* flags) live in mm_types.h.
 */
#pragma once
#include <learnix/mm/mm_types.h>

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

/*
 * mm.h declares the lifecycle methods for a memory descriptor,
 * the unit that owns one whole address space.
 */
#pragma once
#include <learnix/mm/mm_types.h>

/* Allocates a fresh memory descriptor: a new page table with the
 * kernel mappings and an empty VMA list. */
struct mm_desc *mm_alloc (void);

/* Duplicates the memory descriptor old into new. Main use in fork()
 * because we need to copy VMAs. */
int mm_copy(struct mm_desc *old, struct mm_desc *new);

/* Frees every VMA, destroys the page table and finally frees mm. */
void mm_free (struct mm_desc *mm);

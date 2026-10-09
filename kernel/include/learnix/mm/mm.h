/*
 * mm.h declares the lifecycle methods for a memory descriptor,
 * the unit that owns one whole address space.
 */
#pragma once
#include <learnix/mm/mm_types.h>

/* Allocates a fresh memory descriptor: a new page table with the
 * kernel mappings and an empty VMA list. */
struct mm_desc *mm_alloc (void);

/* Frees every VMA, destroys the page table and finally frees mm. */
void mm_free (struct mm_desc *mm);

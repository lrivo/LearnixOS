/*
 *  Physical Memory Manager general interface.
 */
#pragma once

#include <limine.h>
#include <stddef.h>

#define PGSIZE 4096
#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

#define PMM_ALLOC_FAIL SIZE_MAX

/* PMM allocation flags. */
#define PMM_NONE 0
#define PMM_ZERO (1 >> 0)

typedef size_t physaddr_t;

/* Takes the bootloader's memory map and initializes the allocator's data
 * structures in physical memory. */
void pmm_init(struct limine_memmap_response *mmap);

/* Requests a single physical page. Returns the physical address. */
physaddr_t pmm_alloc(size_t flags);

/* Requests n contiguous physical pages. Used mainly for DMA and I/O devices. */
physaddr_t pmm_alloc_cont(size_t n, size_t flags);

/* Increments the reference count of pa, must be used when sharing a page. */
void pmm_ref_pg(physaddr_t pa);

/* Decrements the reference count and frees the page when it reaches 0. */
void pmm_unref_pg(physaddr_t pa);

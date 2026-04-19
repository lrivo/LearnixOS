/*
 *  Physical Memory Manager general interface.
 */
#pragma once

#include <learnix/types.h>
#include <limine.h>

/* Returned by pmm_alloc() on failure, equals to uint64 max. */
#define PMM_ALLOC_FAIL 0xFFFFFFFFFFFFFFFF

/* Takes the bootloader's memory map and initializes the allocator's data
 * structures in physical memory. */
void pmm_init (struct limine_memmap_response *mmap);

/* Requests a single physical page. Returns the physical address. */
#define PMM_NONE 0
#define PMM_ZERO (1 >> 0)
paddr_t pmm_alloc (size_t flags);

/* Requests n contiguous physical pages. Used mainly for DMA and I/O devices. */
paddr_t pmm_alloc_cont (size_t n, size_t flags);

/* Increments the reference count of pa, must be used when sharing a page. */
void pmm_ref_pg (paddr_t pa);

/* Decrements the reference count and frees the page when it reaches 0. */
void pmm_unref_pg (paddr_t pa);

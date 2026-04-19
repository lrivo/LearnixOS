/*
 * This header defines the architecture-independent interface for managing
 * virtual memory and abstracts away architecture specific structures from
 * the rest of the kernel.
 *
 * Responsibilities:
 * - provide a single interface for mapping and unmapping virtual pages upon
 *   the architecture specific implementations
 * - be used by kmalloc()
 *
 * Architecture-specific implementations of these functions should be
 * located in the corresponding arch/ directory.
 */
#pragma once
#include <learnix/types.h>

typedef uintptr_t vaddr_t; // virtual address

/* Maps the virtual address va to the physical address pa in the
 * page table rooted at pgtable. */
#define VMM_FLAG_WRITE (1 << 0)   // map the page as writable
#define VMM_FLAG_EXEC (1 << 1)    // map the page as executable (default: NX)
#define VMM_FLAG_USER (1 << 2)    // usermode page
#define VMM_FLAG_NOCACHE (1 << 3) // this page cannot be cached, useful for MMIO
#define VMM_FLAG_GLOBAL (1 << 4)  // cannot be flushed from the TLB
int vmm_map (void *pgtable, vaddr_t va, paddr_t pa, int flags);

/* Unmaps the virtual address va from the page table rooted at pgtable. */
int vmm_unmap (void *pgtable, vaddr_t va);

/* Returns the physical address of va in the page table rooted at pgtable. */
paddr_t vmm_va_to_pa (void *pgtable, vaddr_t va);

/* Returns the HHDM address of the current page table root. */
vaddr_t vmm_get_pgtable ();

/* Swaps the active page table. Used in context switches. */
void vmm_swap_pgtable (void *new_pgtable);

/* Flushes a single virtual address from the TLB. */
void vmm_flush_single (vaddr_t va);

/* Flushes the entire TLB. */
void vmm_flush_all (void);

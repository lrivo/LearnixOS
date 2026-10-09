/*
 * mm_types.h groups every memory management type definition:
 * VMAs, the memory descriptor and their flags.
 * It declares no function, the prototypes live in vma.h and mm.h.
 */
#pragma once
#include <learnix/types.h>

/* Those are the values for vm_area->flags. */
#define VMA_READ       (1UL << 0)
#define VMA_WRITE      (1UL << 1)
#define VMA_EXEC       (1UL << 2)
#define VMA_FILEBACKED (1UL << 3)
#define VMA_STACK      (1UL << 4)

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

/* This struct describes one virtual address space: the page table
 * that implements it, the VMAs that describe it and the layout of
 * the program image mapped into it.
 * The kernel stack is deliberately NOT part of this: it is a
 * per-process resource, not per-address-space. */
struct mm_desc {
    // page table root of this address space (HHDM address)
    vaddr_t pgtable;
    // head of the sorted VMA list
    struct vm_area *vma_head;

    /* image layout, filled by the ELF loader on execve */
    vaddr_t text_start;
    vaddr_t text_end;
    vaddr_t data_start;
    vaddr_t data_end;
    vaddr_t bss_start;
    vaddr_t bss_end;

    /* heap: fixed base and current program break */
    vaddr_t brk_start;
    vaddr_t brk_curr;
};

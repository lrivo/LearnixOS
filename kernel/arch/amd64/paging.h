/*
 *  x86_64 virtual address format:
 *
 *  +-----+------+------+------+-----+--------+
 *  |     | PML4 | PML3 | PML2 | PTE | OFFSET |
 *  +-----+------+------+------+-----+--------+
 *  63   48     39     30     21    12        0
 *
 *  More info at this blog post:
 *  https://blog.zolutal.io/understanding-paging/
 */
#pragma once
#include <learnix/arch/types.h>

#define PML4_IDX(va) (((vaddr_t)(va) >> 39) & 0x1FF)
#define PML3_IDX(va) (((vaddr_t)(va) >> 30) & 0x1FF)
#define PML2_IDX(va) (((vaddr_t)(va) >> 21) & 0x1FF)
#define PML1_IDX(va) (((vaddr_t)(va) >> 12) & 0x1FF)
#define VA_OFFSET(va) ((vaddr_t)(va) & 0x1FF)
#define VADDR_IDXS(p4, p3, p2, p1)                                             \
  (((uint64_t)(p4) << 39) | ((uint64_t)(p3) << 30) | ((uint64_t)(p2) << 21)    \
   | ((uint64_t)(p1) << 12))

// x86_64 page flags (AMD Programmer's Manual 1-5 page 612)
#define PTE_PRESENT (1ULL << 0)   // mark page as present
#define PTE_WRITE (1ULL << 1)     // page is read/write
#define PTE_USER (1ULL << 2)      // userland page
#define PTE_PWT (1ULL << 3)       // write-through
#define PTE_PCD (1ULL << 4)       // cache disable
#define PTE_ACCESSED (1ULL << 5)  // Accessed bit
#define PTE_DIRTY (1ULL << 6)     // Dirty bit
#define PTE_HUGE (1ULL << 7)      // Page Size (PS) bit
#define PTE_GLOBAL (1ULL << 8)    // Global (G) bit
#define PTE_NX (1ULL << 63)       // No-Execute (NX) bit (if set no code execution)

#define PTE_PA_MASK 0x000FFFFFFFFFF000ULL

typedef uint64_t pml4_t; // Page Global Directory (PGD)
typedef uint64_t pml3_t; // Page Upper Directory (PUD)
typedef uint64_t pml2_t; // Page Middle Directory (PMD)
typedef uint64_t pte_t;  // Page Table Entry

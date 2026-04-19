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

#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITE (1ULL << 1)
#define PTE_PWT (1ULL << 3) // write-through
#define PTE_PCD (1ULL << 4) // cache disable
#define PTE_USER (1ULL << 2)
#define PTE_HUGE (1ULL << 7)
#define PTE_PA_MASK 0x000FFFFFFFFFF000ULL

typedef uint64_t pml4_t; // Page Global Directory (PGD)
typedef uint64_t pml3_t; // Page Upper Directory (PUD)
typedef uint64_t pml2_t; // Page Middle Directory (PMD)
typedef uint64_t pte_t;  // Page Table Entry

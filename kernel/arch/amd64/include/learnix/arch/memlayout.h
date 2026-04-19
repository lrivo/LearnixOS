/*
 *  x86_64 memory layout constants and helper functions.
 */
#pragma once
#include "types.h"

/* Kernel code & data linker symbols. */
extern char _kernel_code_start[];
extern char _kernel_code_end[];
extern char _kernel_data_start[];
extern char _kernel_data_end[];
extern char _kernel_bss_start[];
extern char _kernel_bss_end[];

/* Runtime-filled addresses from Limine. */
extern vaddr_t hhdm_offset;
extern vaddr_t kernel_virt_base;
extern paddr_t kernel_phys_base;

/* Known/constant addresses */
// TODO: remember to not hardcode this when KASLR will be on
#define KMALLOC_START 0xFFFF808000000000ULL // pml4[257]
#define LAPIC_VIRT_BASE 0xFFFFFFFF80000000ULL

/* Page frames math. */
#define PGSIZE 4096
#define PGROUNDUP(a) (((a) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

/* HHDM translation macros. */
#define P2V(pa) ((uintptr_t)(pa) + (hhdm_offset))
#define V2P(va) ((uintptr_t)(va) - (hhdm_offset))

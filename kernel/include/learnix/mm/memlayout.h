#pragma once

#include <learnix/types.h>

/* Kernel code & data linker symbols. */
extern char _kernel_code_start[];
extern char _kernel_code_end[];
extern char _kernel_data_start[];
extern char _kernel_data_end[];
extern char _kernel_bss_start[];
extern char _kernel_bss_end[];

/* Runtime-filled addresses from Limine. */
extern uintptr_t hhdm_offset;
extern uintptr_t kernel_virt_base;
extern uintptr_t kernel_phys_base;

// TODO move in pmm.h
/* Page Frames math. */
#define PGSIZE 4096
#define PGROUNDUP(a) (((a) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

/* Address translation macros. */
#define PA_TO_HHDM(pa) ((uintptr_t)(pa) + (hhdm_offset))
#define HHDM_TO_PA(va) ((uintptr_t)(va) - (hhdm_offset))

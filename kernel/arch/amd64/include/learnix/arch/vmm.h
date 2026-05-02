#pragma once
#include "types.h"
#include "memlayout.h"

int arch_pg_map(vaddr_t pgtable, vaddr_t va, paddr_t pa, int flags);

int arch_pg_unmap(vaddr_t pgtable, vaddr_t va);

paddr_t arch_va_to_pa(vaddr_t pgtable, vaddr_t va);

static inline vaddr_t
arch_get_pgtable(void)
{
  vaddr_t val;
  asm volatile ("mov %%cr3,%0" : "=r"(val));
  return (vaddr_t)P2V (val & ~0xFFFULL);
}

static inline void
arch_tlb_flush(vaddr_t va)
{
  asm volatile ("invlpg (%0)" ::"r"(va) : "memory");
}

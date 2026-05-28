#include <learnix/mm/vmm.h>
#include <learnix/arch/vmm.h>
#include <learnix/arch/memlayout.h>

// need this for proc_init() to copy kernel mappings
static vaddr_t kern_pgtable;

void
vmm_init(void)
{
  // save Limine's kernel page table root
  kern_pgtable = arch_get_pgtable();
}

int
vmm_map(vaddr_t pgtable, vaddr_t va, paddr_t pa, int flags)
{
  return arch_pg_map(pgtable, va, pa, flags);
}

int 
vmm_unmap(vaddr_t pgtable, vaddr_t va)
{
  return arch_pg_unmap(pgtable, va);
}

int
vmm_map_range(vaddr_t pgtable, vaddr_t va, paddr_t pa, int flags, size_t n)
{
  for (size_t i = 0; i < n; i++, va += PGSIZE, pa += PGSIZE)
  {
    if (arch_pg_map(pgtable, va, pa, flags) < 0)
      return -1;
  }
  return 0;
}

int
vmm_unmap_range(vaddr_t pgtable, vaddr_t va, size_t n)
{
  for (size_t i = 0; i < n; i++, va += PGSIZE)
  {
    if (arch_pg_unmap(pgtable, va) < 0)
      return -1;
  }
  return 0;
}

paddr_t
vmm_va_to_pa(vaddr_t pgtable, vaddr_t va)
{
  return arch_va_to_pa(pgtable, va);
}

vaddr_t
vmm_get_kern_pgtable(void)
{
  return kern_pgtable;
}

vaddr_t
vmm_get_pgtable(void)
{
  return arch_get_pgtable();
}

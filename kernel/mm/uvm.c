#include <learnix/mm/vmm.h>
#include <learnix/mm/pmm.h>
#include <learnix/lib/string.h>
#include <learnix/arch/memlayout.h>

vaddr_t
uvm_alloc()
{
  // get a physical page
  vaddr_t pgtable = P2V(pmm_alloc(PMM_NONE));

  // copy kernel mappings
  memcpy((void*)pgtable, (void*)vmm_get_kern_pgtable(), PGSIZE);

  return pgtable;
}

void
uvm_copy(vaddr_t dest, vaddr_t src)
{
  // TODO: just calls arch_copyuvm
}

void
uvm_free(vaddr_t uvm)
{
  // TODO: recursively free userspace mappings
  // and intermediate levels too
}

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

int
uvm_copy(vaddr_t dest, vaddr_t src)
{
  if (dest == 0 || src == 0)
    return -1;

  if (dest != src)
    arch_uvm_copy_or_destroy(dest, src, false); 

  return 0;
}

int
uvm_destroy(vaddr_t pgtable)
{
  if (pgtable == 0)
    return -1;
  
  // walk and destroy all userspace mappings
  arch_uvm_copy_or_destroy(0, pgtable, true);

  // free the page table root
  pmm_unref_pg(V2P(pgtable));

  return 0;
}

#include "learnix/arch/types.h"
#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/lib/elf.h>
#include <learnix/lib/string.h>
#include <learnix/lib/kprintf.h>
#include <learnix/arch/memlayout.h>

/* Tries to map a PT_LOAD segment */
static int
elf_seg_loadable(struct elf64_hdr *hdr, struct elf64_phdr *phent, vaddr_t pgdir)
{
  size_t npages, i, n;
  paddr_t pa;
  int flags = VMM_FLAG_USER;

  // how many virtual pages are needed for this section (ceil roundup)
  npages = ((size_t)phent->p_memsz + PGSIZE - 1) / PGSIZE;

  /* map pg with the section's requested flags.
   * by default this flags means user readable. */
  if (phent->p_flags & PF_W)
    flags |= VMM_FLAG_WRITE;
  if (phent->p_flags & PF_X)
    flags |= VMM_FLAG_EXEC;
  if (phent->p_flags & PF_W && phent->p_flags & PF_X)
    return -1;  // respect W^X policy

  // FIMME: bad solution for multi-page sections
  for (i = 0; i < npages; i++) {
      // get a physical page
      pa = pmm_alloc(PMM_ZERO);
      if (pa == PMM_ALLOC_FAIL)
          return -1;    // TODO goto cleanup

      // always copy PGSIZE unless we are on the last page
      if (i < npages - 1) {
          n = PGSIZE;
       } else {
           n = npages == 1 ? phent->p_filesz : phent->p_filesz - (PGSIZE * (npages - 1));
       }

      // copy the section's byte into the page
      memcpy((void*)P2V(pa), (void*)hdr + phent->p_offset + (i * PGSIZE), n);

      // insert the mapping
      if (vmm_map(pgdir, (vaddr_t)phent->p_vaddr + (i * PGSIZE), pa, flags) < 0)
        return -1;  // TODO goto cleanup
  }

  return 0;
}

/* Load the userspace stack into pgdir at a fixed address. */
static int
elf_seg_stack(struct elf64_hdr *hdr, struct elf64_phdr *phent, vaddr_t pgdir)
{
  paddr_t pg = pmm_alloc(PMM_ZERO);

  return vmm_map(pgdir, (vaddr_t)USR_STACK, pg,
           VMM_FLAG_WRITE | VMM_FLAG_USER);
}

int
elf_validate(struct elf64_hdr *hdr)
{
  // file not found
  if (hdr == NULL)
    return ENOEXEC;

  // the first 4 bytes must match ELF_MAGIC
  if (memcmp(hdr->e_ident, ELF_MAGIC, 4) != 0)
    return ENOEXEC;

  // must be an executable ELF file
  if (hdr->e_type != ET_EXEC)
    return ENOEXEC;

  // must be for the current architecture
  if (hdr->e_machine != EM_AMD64) // FIXME
    return -1;

  return 0;
}

int
elf_load(struct elf64_hdr *hdr, vaddr_t pgdir)
{
  int ret = 0;
  // find the Program Header Table
  struct elf64_phdr *phent = (struct elf64_phdr*)((vaddr_t)hdr + (vaddr_t)hdr->e_phoff);

  for (int i = 0; i < hdr->e_phnum; i++, phent++)
  {
    // try to map the current entry
    switch (phent->p_type)
    {
      case PT_LOAD:
        ret = elf_seg_loadable(hdr, phent, pgdir);
        break;
      case PT_STACK:
        ret = elf_seg_stack(hdr, phent, pgdir);
        break;
      default:
        continue;
    }

    // return error if a section mapping failed
    if (ret < 0)
      return ret;
  }

  return 0;
}

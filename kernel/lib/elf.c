#include <learnix/mm/pmm.h>
#include <learnix/mm/vmm.h>
#include <learnix/lib/elf.h>
#include <learnix/lib/string.h>
#include <learnix/arch/memlayout.h>

/* Tries to map a PT_LOAD segment */
static int
elf_seg_loadable(struct elf64_hdr *hdr, struct elf64_phdr *phent, vaddr_t pgdir)
{
  // request a physical page for this ELF segment 
  vaddr_t pg = P2V(pmm_alloc(PMM_ZERO));

  // copy the section's bytes into pg
  memcpy((void*)pg, (void*)hdr + phent->p_offset, phent->p_filesz);

  // zero the bss section
  if (phent->p_memsz > phent->p_filesz)
    memset((void*)pg + phent->p_filesz, 0, phent->p_memsz - phent->p_filesz);
    
  /* map pg with the section's requested flags.
   * by default this flags means user readable. */
  int flags = VMM_FLAG_USER;
  if (phent->p_flags & PF_W)
    flags |= VMM_FLAG_WRITE;
  if (phent->p_flags & PF_X)
    flags |= VMM_FLAG_EXEC;
  if (phent->p_flags & PF_W && phent->p_flags & PF_X)
    return -1;  // respect W^X policy

  // insert the mapping into pgdir
  if (vmm_map(pgdir, (vaddr_t)phent->p_vaddr, V2P(pg), flags) < 0)
    return -1;

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
  if (hdr->e_machine != EM_AMD64) // FIXME:
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

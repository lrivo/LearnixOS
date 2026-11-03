#include "learnix/arch/types.h"
#include <learnix/mm/mm.h>
#include <learnix/mm/pmm.h>
#include <learnix/mm/vma.h>
#include <learnix/mm/vmm.h>
#include <learnix/lib/elf.h>
#include <learnix/lib/string.h>
#include <learnix/lib/kprintf.h>
#include <learnix/arch/memlayout.h>

/* Tries to map a PT_LOAD segment. Uses lazy paging, we only create VMAs and leave
 * the actual mapping at page fault time. */
int
elf_seg_loadable(struct elf64_hdr *hdr, struct elf64_phdr *phent, struct mm_desc *mm)
{
  // prepare VMA flags for this ELF section (FILEBACKED)
  uint64_t vma_flags = VMA_FILEBACKED;
  if (phent->p_flags & PF_R)
    vma_flags |= VMA_READ;
  if (phent->p_flags & PF_W)
    vma_flags |= VMA_WRITE;
  if (phent->p_flags & PF_X)
    vma_flags |= VMA_EXEC;

  // insert the VMA
  struct vm_area *vma = vma_alloc(
      (vaddr_t)phent->p_vaddr,
      (vaddr_t)phent->p_vaddr + phent->p_memsz,
      vma_flags
  );
  if (!vma || !vma_insert(&mm->vma_head, vma))
    return -1;

  /* vma->backing is just the virtual address of the Limine file. Must be
   * replaced with a struct inode* to allow fetching disk files. */
  vma->backing = (vaddr_t)hdr;
  // byte offset from the start of the file
  vma->backing_offset = phent->p_offset;
  // how many bytes to actually copy (the rest is assumed zero)
  vma->backing_size = phent->p_filesz;

  return 0;
}

/* Load the userspace stack into pgdir at a fixed address. */
static int
elf_seg_stack(struct elf64_hdr *hdr, struct elf64_phdr *phent, struct mm_desc *mm)
{
  paddr_t pg = pmm_alloc(PMM_ZERO);

  int ret = vmm_map(mm->pgtable, (vaddr_t)USR_STACK, pg, VMM_FLAG_WRITE | VMM_FLAG_USER);
  if (ret < 0)
    return ret;

  /* the stack is not backed by any file: plain anonymous VMA */
  struct vm_area *vma = vma_alloc((vaddr_t)USR_STACK,
                                  (vaddr_t)USR_STACK + PGSIZE,
                                  VMA_READ | VMA_WRITE | VMA_STACK);
  if (!vma || !vma_insert(&mm->vma_head, vma))
    return -1;

  return 0;
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
elf_load(struct elf64_hdr *hdr, struct mm_desc *mm)
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
        ret = elf_seg_loadable(hdr, phent, mm);
        break;
      case PT_STACK:
        ret = elf_seg_stack(hdr, phent, mm);
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

#include "learnix/arch/types.h"
#include "learnix/lib/debug.h"
#include "learnix/mm/pmm.h"
#include "learnix/mm/vmm.h"
#include <learnix/arch/memlayout.h>
#include <learnix/cpu.h>
#include <learnix/elf.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
#include <learnix/process.h>
#include <learnix/syscall.h>
#include <limine.h>

#define ENOEXEC -2

__attribute ((
    used,
    section (".limine_requests"))) static volatile struct limine_module_request
    module_request = { .id = LIMINE_MODULE_REQUEST_ID, .revision = 5 };

static size_t
elf_validate (struct elf64_hdr *hdr)
{
  // the first 4 bytes must match ELF_MAGIC
  if (memcmp (hdr->e_ident, ELF_MAGIC, 4) != 0)
    return ENOEXEC;

  return 0;
}

void
sys_execve (struct intr_trap_frame *tf)
{
  size_t ret;               // return code
  vaddr_t uvm = 0;          // new page table
  struct process *curr;     // process that called execve()
  struct elf64_hdr *elf;    // ELF header
  struct elf64_phdr *phent; // ELF program header table

  // TEST: this always loads /boot/hello
  struct limine_module_response *modules = module_request.response;
  struct limine_file *file = (struct limine_file *)modules->modules[0];
  elf = (struct elf64_hdr *)file->address;

  // validate the ELF program
  ret = elf_validate (elf);
  if (ret < 0)
    goto bad;

  // allocate a new page table with kernel mappings
  uvm = uvm_alloc ();

  // walk the Program Header Table
  phent = (struct elf64_phdr *)((vaddr_t)elf + (vaddr_t)elf->e_phoff);
  for (int i = 0; i < (int)elf->e_phnum; i++, phent++)
  {
    if (phent->p_type != PT_LOAD)
      continue;
    if (phent->p_memsz < phent->p_filesz)
      goto bad;
    if (phent->p_vaddr % PGSIZE != 0)
      goto bad;

    // TODO: all of this can be incapsulated in uvm_map()
    // request a physical page
    vaddr_t pg = P2V (pmm_alloc (PMM_ZERO));

    // TODO: also account for p_memsz for .bss
    // copy the content
    memcpy ((void *)pg, (void *)elf + phent->p_offset, phent->p_filesz);
    dbg_hexdump ((void *)pg, 2);

    // mapping flags
    int flags = VMM_FLAG_USER;
    if (phent->p_flags & PF_W)
      flags |= VMM_FLAG_WRITE;
    if (phent->p_flags & PF_X)
      flags |= VMM_FLAG_EXEC;

    vmm_map(uvm, phent->p_vaddr, V2P (pg), flags);
  }
  
  // TODO: free the old curr->pgtable

  // now we can modify the PCB of the current process
  curr = arch_cpu_get()->proc;

  // swap curr's page table
  curr->pgtable = uvm;
  
  // allocate a usermode stack
  vmm_map(uvm, 0x7ffffffdd000UL, pmm_alloc(PMM_ZERO), VMM_FLAG_WRITE | VMM_FLAG_USER);

  // reinitialize curr's trap frame
  arch_proc_init(curr, elf->e_entry, 0x7ffffffdd000UL + PGSIZE);
  
  // FIXME: must be moved in a vmm function
  asm volatile ("mov %0,%%cr3" :: "r"(V2P(uvm)));
  
  // return the the new process, as we've modified the trap frame
  return;

  /* we only arrive here on error, because
   * a successful execve() never returns. */
bad:
  if (uvm)
    pmm_unref_pg (V2P (uvm));
  RET (tf) = ret;
}

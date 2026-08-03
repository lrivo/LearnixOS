#include "learnix/mm/vmm.h"
#include <learnix/arch/memlayout.h>
#include <learnix/cpu.h>
#include <learnix/lib/elf.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/string.h>
#include <learnix/process.h>
#include <learnix/syscall.h>
#include <learnix/lib/limine_module.h>

void
sys_execve (struct intr_trap_frame *tf)
{
  ssize_t ret;                                      // return code
  vaddr_t uvm = 0;                                  // new page table
  struct process *curr = arch_cpu_get()->proc;      // process that called execve()
  struct elf64_hdr *elf;                            // ELF header

  // get the ELF file
  struct limine_file *f = limine_module_get((const char*)ARG0(tf));
  if (!f) {
    ret = -1;
    goto bad;
  }
  elf = (struct elf64_hdr*)f->address;

  // validate the ELF program
  ret = elf_validate (elf);
  if (ret < 0)
    goto bad;

  // allocate a new page table with kernel mappings
  uvm = uvm_alloc ();

  // try to load the ELF image into uvm
  ret = elf_load(elf, uvm);
  if (ret < 0)
    goto bad;

  // destroy the old uvm (TODO: make this deferred when kernel_idle runs)
  uvm_destroy(curr->pgtable);

  // swap curr's page table
  curr->pgtable = uvm;

  /* this function modifies curr->tf in place, when sys_dispatcher
   * returns and eventually calls kernel_exit it will land on this
   * modified trap frame for the new process's image.
   * It also updates the page table root (and TSS on x86_64). */
  arch_proc_exec(curr, elf->e_entry, (vaddr_t)USR_STACK + PGSIZE);

  // unwind the syscall stack frame
  return;

  /* if the requested ELF file was bad we destroy the
   * new page table and return an error to the caller. */
bad:
  if (uvm)
    uvm_destroy(uvm);
  RET (tf) = ret;
}

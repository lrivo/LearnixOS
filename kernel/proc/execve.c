#include <learnix/mm/mm.h>
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
  struct mm_desc *mm = NULL;                        // new address space
  struct process *curr = arch_cpu_get()->proc;      // process that called execve()
  struct elf64_hdr *elf;                            // ELF header
  char path[PATH_MAX];                              // kernel copy of ARG0

  // pathname is a userspace pointer: copy it into kernel memory first
  ret = arch_strncpy_from_user (path, (const char*)ARG0 (tf), sizeof (path));
  if (ret < 0)
    goto bad;   // this returns -EFAULT

  // get the ELF file
  struct limine_file *f = limine_module_get (path);
  if (!f) {
    ret = -1;
    goto bad;
  }
  elf = (struct elf64_hdr*)f->address;

  // validate the ELF program
  ret = elf_validate (elf);
  if (ret < 0)
    goto bad;

  // allocate a new address space (page table + empty VMA list)
  mm = mm_alloc ();

  // try to load the ELF image into it
  ret = elf_load(elf, mm->pgtable);
  if (ret < 0)
    goto bad;

  // destroy the old address space (TODO: make this deferred when kernel_idle runs)
  mm_free (curr->mm);

  // swap curr's address space
  curr->mm = mm;

  /* this function modifies curr->tf in place, when sys_dispatcher
   * returns and eventually calls kernel_exit it will land on this
   * modified trap frame for the new process's image.
   * It also updates the page table root (and TSS on x86_64). */
  arch_proc_exec(curr, elf->e_entry, (vaddr_t)USR_STACK + PGSIZE);

  // unwind the syscall stack frame
  kprintfdbg("[sys_execve] PID %d\n", (int)curr->pid);
  return;

  /* if the requested ELF file was bad we destroy the
   * new address space and return an error to the caller. */
bad:
  if (mm)
    mm_free (mm);
  RET (tf) = ret;
}

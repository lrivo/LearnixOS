/*
 *  x86_64 arch functions for processes.
 */
#include "gdt.h"
#include "learnix/arch/types.h"
#include <learnix/lib/string.h>
#include <learnix/process.h>

void
arch_proc_init (struct process *p, vaddr_t user_ip, vaddr_t user_sp)
{
  /* initialize the p->tf pointer */
  p->tf = (struct intr_trap_frame *)((vaddr_t)p->kstack + KSTACK_SIZE
                                     - sizeof (struct intr_trap_frame));

  /* we set all registers to 0 on first run */
  memset ((void *)p->tf, 0x00, sizeof (struct intr_trap_frame));

  /* iretq frame setup */
  p->tf->rip = user_ip;
  p->tf->cs = GDT_UCODE | 3;
  p->tf->rflags = 0x202; // IF | BRKI
  p->tf->rsp = user_sp;
  p->tf->ss = GDT_UDATA | 3;
}

void
arch_context_switch (struct process *prev, struct process *next)
{
  // switch TSS.rsp0
  tss_set_rsp0 ((vaddr_t)next->kstack + KSTACK_SIZE);
}

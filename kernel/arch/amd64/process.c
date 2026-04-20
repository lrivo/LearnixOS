/*
 *  x86_64 arch functions for processes.
 */
#include "gdt.h"
#include <learnix/arch/types.h>
#include <learnix/arch/memlayout.h>
#include <learnix/lib/string.h>
#include <learnix/process.h>

/* This is an assembly trampoline function that should only
   be called the first time a process gets scheduled. */
extern void jump_usr_first_time(void);

/* This swaps the kernel stack from prev to next */
extern void _switch_to(struct arch_proc_context *prev, struct arch_proc_context *next);

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
  p->tf->rflags = 0x202; // IF (allow ring3->ring0 intr) | BRKI
  p->tf->rsp = user_sp;
  p->tf->ss = GDT_UDATA | 3;
  
  /* put the jump_user_first_time trampoline
     before the trap frame. */
  uint64_t *sp = (uint64_t*)p->tf;
  
  // this is the return address for _switch_to
  *(--sp) = (uint64_t)jump_usr_first_time;
  *(--sp) = 0;  // rbx 
  *(--sp) = 0;  // rsp
  *(--sp) = 0;  // r12
  *(--sp) = 0;  // r13
  *(--sp) = 0;  // r14
  *(--sp) = 0;  // r15
  
  p->ctx.rsp = (uint64_t)sp;
}

void
arch_context_switch (struct process *prev, struct process *next)
{
  // switch TSS.rsp0
  tss_set_rsp0 ((vaddr_t)next->kstack + KSTACK_SIZE);
  
  /* change address space at the last possible moment
     before context switching as this flushes the TLB. */
  asm volatile ("mov %0,%%cr3" ::"r"(V2P (next->pgtable)));

  // assembly stub that does the actual switch
  _switch_to(&prev->ctx, &next->ctx);
  
  /* we'll re-enter here the next time prev will be
     scheduled. */
}

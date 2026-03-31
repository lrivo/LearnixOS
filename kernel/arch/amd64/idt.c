#include <learnix/arch/arch.h>
#include <learnix/lib/debug.h>
#include <learnix/lib/string.h>
#include <learnix/lib/kpanic.h>
#include <stdint.h>
#include "idt.h"

/* Assembly stubs defined in isr_stubs.asm that are registered directly in the IDT
 * and then calls intr_dispatcher after saving registers and pushing the
 * vector number and error code. */
extern uintptr_t isr_stubs_table[];

/* This is the actual Interrupt Descriptor Table that the CPU see. */
static idtr_t idtr;
static idt_entry_t idt[IDT_ENTRIES];

/* This contains function pointers to the actual handlers for intr_dispatcher
 * to call them. */
static intr_handler_t handlers[IDT_ENTRIES] = { NULL };

// Loads the IDT using the lidt x86 instruction.
static inline void
idt_load ()
{
  idtr.size = (sizeof (idt_entry_t) * IDT_ENTRIES) - 1;
  idtr.offset = (uintptr_t)&idt;
  asm volatile ("lidt (%0)" : : "r"(&idtr));
}

/* https://wiki.osdev.org/Interrupt_Descriptor_Table#Structure_on_x86-64 */
static inline void
idt_set_gate (size_t idx, uintptr_t handler, uint16_t selector, uint8_t ist,
              uint8_t type_attributes)
{
  idt[idx].offset_low = handler & 0xFFFF;         // 16 LSB of handler
  idt[idx].offset_mid = (handler >> 16) & 0xFFFF; // bits 16 to 31 of handler
  idt[idx].offset_high
      = (handler >> 32) & 0xFFFFFFFF; // bits 32 to 63 of handler
  idt[idx].selector = selector;
  idt[idx].ist = ist;
  idt[idx].type_attributes = type_attributes;
  idt[idx].reserved = 0;
}

static void
handler_div_zero (struct intr_stack_frame_t *f)
{
  kpanic ("Division by zero at %p\n", f->rip);
  dbg_print_stack_trace (5);
  arch_hcf ();
}

static void
handler_timer (struct intr_stack_frame_t *f)
{
  arch_interrupts_eoi (0);
}

void
arch_interrupts_register (size_t vector, intr_handler_t handler,
                          intr_flags_t flags)
{
  // 0. Input checks
  if (vector > IDT_ENTRIES)
    return;

  // 1. Register the handler in the dispatcher's table
  handlers[vector] = handler;

  // 2. Update the IDT, in case the flags changed
  idt_set_gate (vector, isr_stubs_table[vector], KERN_CODE_SEGMENT, 0,
                INTERRUPT_GATE);
}

void
intr_dispatcher (struct intr_stack_frame_t *frame)
{
  int vector_num = (int)frame->vector_num;
  
  if (handlers[vector_num])
    handlers[vector_num](frame);
  else
    kpanic("intr_dispatcher: %d is not registered", vector_num);
}

void
idt_init ()
{
  // register the assembly stubs with an empty handler.
  for (int i = 0; i <= 33; i++)
  {
    arch_interrupts_register (i, NULL, INTR_FLAG_DEFAULT);
  }
  
  // load the actual handlers
  arch_interrupts_register (0, handler_div_zero, INTR_FLAG_DEFAULT);
  arch_interrupts_register (0x20, handler_timer, INTR_FLAG_DEFAULT);

  idt_load ();
}

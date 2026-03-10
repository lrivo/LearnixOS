#include <arch.h>
#include <interrupts.h>
#include <lib/string.h>
#include <stdint.h>
#include "idt.h"

/* Assembly interrupt stubs that call the dispatcher after saving registers and pushing the vector number. */
extern uintptr_t isr_stubs_table[];

/* Dispatcher table that contains function pointers to the actual handlers. */
static intr_handler_t handlers[IDT_ENTRIES] = {NULL};

static idtr_t idtr;

/* The actual Interrupt Descriptor Table that we load in the lidt special register. */
static idt_entry_t idt[IDT_ENTRIES];

static inline void idt_load() {
  idtr.size = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
  idtr.offset = (uintptr_t)&idt;
  asm volatile("lidt (%0)" : : "r"(&idtr));
}

/* https://wiki.osdev.org/Interrupt_Descriptor_Table#Structure_on_x86-64 */
static void idt_set_gate(size_t idx, uintptr_t handler, uint16_t selector, uint8_t ist, uint8_t type_attributes) {
  idt[idx].offset_low = handler & 0xFFFF;         // 16 LSB of handler
  idt[idx].offset_mid = (handler >> 16) & 0xFFFF; // bits 16 to 31 of handler
  idt[idx].offset_high =
      (handler >> 32) & 0xFFFFFFFF; // bits 32 to 63 of handler
  idt[idx].selector = selector;
  idt[idx].ist = ist;
  idt[idx].type_attributes = type_attributes;
  idt[idx].reserved = 0;
}

static void handler_div_zero(struct intr_stack_frame_t *f) {
  // TEST: jump to the instruction after the division
  // just to check that I am correctly saving/restoring the regs
  f->rip = 0xffffffff80001032;
}

void arch_interrupts_register(size_t vector, intr_handler_t handler, intr_flags_t flags) {
  // 0. Input checks
  if (vector > IDT_ENTRIES || !handler) 
    return;

  // 1. Register the handler in the dispatcher's table
  handlers[vector] = handler;

  // 2. Update the IDT
  idt_set_gate(vector, isr_stubs_table[vector], KERN_CODE_SEGMENT, 0, INTERRUPT_GATE);
}

void intr_dispatcher(struct intr_stack_frame_t *frame) {
  uint64_t vector_num = frame->vector_num;

  if (handlers[vector_num]) {
    handlers[vector_num](frame);
  }
}

void idt_init() {
  memset(idt, 0, sizeof(idt_entry_t) * IDT_ENTRIES);

  arch_interrupts_register(0, handler_div_zero, INTR_FLAG_DEFAULT);

  idt_load();
}

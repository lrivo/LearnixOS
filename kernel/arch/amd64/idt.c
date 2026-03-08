#include <interrupts.h>
#include <lib/string.h>
#include <stdint.h>
#include "idt.h"

static idtr_t idtr;
static idt_entry_t idt[IDT_ENTRIES];

static inline void idt_load() {
  idtr.size = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
  idtr.offset = (uintptr_t)&idt;
  asm volatile("lidt (%0)" : : "r"(&idtr));
}

/* https://wiki.osdev.org/Interrupt_Descriptor_Table#Structure_on_x86-64 */
static void idt_set_gate(size_t idx, uintptr_t handler, uint16_t selector,
                         uint8_t ist, uint8_t type_attributes) {
  idt[idx].offset_low = handler & 0xFFFF;         // 16 LSB of handler
  idt[idx].offset_mid = (handler >> 16) & 0xFFFF; // bits 16 to 31 of handler
  idt[idx].offset_high =
      (handler >> 32) & 0xFFFFFFFF; // bits 32 to 63 of handler
  idt[idx].selector = selector;
  idt[idx].ist = ist;
  idt[idx].type_attributes = type_attributes;
  idt[idx].reserved = 0;
}

void idt_init() {
  memset(idt, 0, sizeof(idt_entry_t) * IDT_ENTRIES);
  
  idt_load();
}

void arch_interrupts_register(size_t vector, void *handler, intr_flags_t flags) {
  // default values
  uint16_t selector = 0x08;   // GDT's kernel code segment
  uint8_t ist = 0;
  uint8_t type_attributes = INTERRUPT_GATE;

  // tune them based on the given flags
  if (flags == INTR_FLAG_TRAP) {
    type_attributes = TRAP_GATE;
  }

  if (flags == INTR_FLAG_USR) {
    type_attributes |= (1 << 5) | (1 << 6);  // DPL = 3
  }


  idt_set_gate(vector, (uintptr_t)handler, selector, ist, type_attributes);
}

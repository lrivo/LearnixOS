#pragma once
#include <learnix/arch/interrupts.h>

#define IDT_ENTRIES 256
#define KERN_CODE_SEGMENT 8
#define INTERRUPT_GATE 0x8E
#define TRAP_GATE 0x8F

typedef void (*intr_handler_t) (struct intr_trap_frame *tf);

/* IDT descriptor structure on x86_64 */
typedef struct
{
  uint16_t size;
  uintptr_t offset;
} __attribute__ ((packed)) idtr_t;

/* IDT Gate Descriptor on x86_64 */
typedef struct
{
  uint16_t offset_low;
  uint16_t selector;
  uint8_t ist;             // only the 4 LSB
  uint8_t type_attributes; // gate type, DPL, P
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t reserved;
} __attribute__ ((packed)) idt_entry_t;

/* Called by arch_stage_1() to initialize the idt */
void idt_init ();

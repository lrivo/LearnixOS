#pragma once
#include <interrupts.h>
#include <stdint.h>

#define IDT_ENTRIES 256
#define KERN_CODE_SEGMENT 8
#define INTERRUPT_GATE 0x8E
#define TRAP_GATE 0x8F

struct intr_stack_frame_t
{
  // saved registers
  uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi;
  uint64_t r8, r9, r10, r11, r12, r13, r14, r15;

  uint64_t vector_num;

  // automatically pushed by the CPU
  uint64_t error; // OPTIONAL
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} __attribute__ ((packed));

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

#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include <arch.h>

void arch_stage_1() {
  gdt_init();
  idt_init();
  pic_init();
}

void arch_hcf() {
  for (;;) {
    asm volatile("hlt");
  }
}

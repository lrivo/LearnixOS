#include "idt.h"
#include <arch.h>

void arch_stage_1() {
  idt_init();
}

void arch_hcf() {
  for (;;) {
    asm volatile("hlt");
  }
}

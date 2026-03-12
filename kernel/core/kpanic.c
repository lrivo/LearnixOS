#include <arch.h>
#include <drivers/console.h>
#include <interrupts.h>

void kernel_panic(const char *msg) {
  // disable interrupts
  arch_interrupts_disable();

  // output the error on screen in red
  console_set_color(0, 0xFF0000);
  console_putstr("\n** KERNEL PANIC **\n");
  console_putstr(msg);

  // halt the CPU
  arch_hcf();
}

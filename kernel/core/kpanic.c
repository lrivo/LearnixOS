#include <learnix/arch/arch.h>
#include <learnix/arch/interrupts.h>
#include <learnix/drivers/console/console.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/kpanic.h>
#include <stdarg.h>

void
kpanic (const char *fmt, ...)
{
  // disable interrupts
  arch_interrupts_disable ();

  // output the error on screen in red
  console_set_color (0, 0xFF0000);
  kprintf ("\n** KERNEL PANIC **\n");

  // handles format arguments
  va_list args;
  va_start (args, fmt);
  kvprintf (fmt, args);
  va_end (args);

  // halt the CPU
  arch_hcf ();
}

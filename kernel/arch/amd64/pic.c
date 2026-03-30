#include "pic.h"
#include "drivers/console.h"
#include <io.h>
#include <lib/kprintf.h>
#include <stdint.h>

#define IO_WAIT pio_write8 (0x80, 0)

void
pic_init (void)
{
  /*
   * UEFI automatically starts with the APIC enabled so we must disable it for
   * the legacy 8259 PIC to work. We do this by clearing the 11th bit of the MSR
   * register.
   */
  uint32_t lo, hi;
  asm volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0x1B));
  lo &= ~(1 << 11); // clear APIC enable bit
  asm volatile ("wrmsr" : : "a"(lo), "d"(hi), "c"(0x1B));

  // ICW1: start the initialization sequence (in cascade mode)
  pio_write8 (PIC1_CMD, ICW1_INIT | ICW1_ICW4);
  IO_WAIT;
  pio_write8 (PIC2_CMD, ICW1_INIT | ICW1_ICW4);
  IO_WAIT;

  // ICW2: configure PIC's remapped offset for protected mode
  pio_write8 (PIC1_DATA, 0x20);
  IO_WAIT;
  pio_write8 (PIC2_DATA, 0x28);
  IO_WAIT;

  // ICW3: tell master pic that there is a slave PIC at IRQ 2
  pio_write8 (PIC1_DATA, 1 << 0x2);
  IO_WAIT;
  // ICW3: tell slave PIC its cascade identity
  pio_write8 (PIC2_DATA, 0x2);

  // ICW4: force the PICs to use the 8086 mode
  pio_write8 (PIC1_DATA, ICW4_8086);
  IO_WAIT;
  pio_write8 (PIC2_DATA, ICW4_8086);
  IO_WAIT;

  // mask everything on both PICs
  pio_write8 (PIC1_DATA, 0xFF);
  pio_write8 (PIC2_DATA, 0xFF);

  // FIXME: move from here, just for testing now
  pic_edit_mask (1, 0); // keyboard ISR
}

void
pic_edit_mask (int irq, int set)
{
  uint16_t port;
  uint8_t mask;

  // select the correct PIC
  if (irq < 8)
  {
    port = PIC1_DATA;
  }
  else
  {
    port = PIC2_DATA;
    irq -= 8;
  }

  // set or clear the corresponding bit
  mask = pio_read8 (port);
  if (set)
    mask |= (1 << irq);
  else
    mask &= ~(1 << irq);

  // write the new mask
  pio_write8 (port, mask);
}

void
pic_eoi (int irq)
{
  if (irq >= 8)
    pio_write8 (PIC2_CMD, EOI);
  pio_write8 (PIC1_CMD, EOI);
}

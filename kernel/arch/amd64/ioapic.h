#pragma once
#include <learnix/arch/types.h>

#define IOAPIC_IRQ_REDTBL(n) (0x10 + 2 * (n))

struct rte
{
  uint64_t vector : 8;
  uint64_t del_mode : 3;
  uint64_t dst_mode : 1;
  uint64_t polarity : 1;
  uint64_t remote_irr : 1;
  uint64_t trigger_mode : 1;
  uint64_t mask : 1;
  uint64_t reserved : 39;
  uint64_t dest : 8;
} __attribute__ ((packed));

void ioapic_init ();

/* Redirects global irq to the specified vector on
 * the calling core's LAPIC. */
void ioapic_irq_redir (uint8_t irq, uint8_t vector);

#include "ioapic.h"
#include "lapic.h"
#include <learnix/acpi.h>
#include <learnix/arch/memlayout.h>
#include <learnix/mm/vmm.h>
#include <learnix/types.h>

// I/O APIC MMIO registers
static volatile uint32_t *regsel, *iowin;

static inline uint32_t
ioapic_read (const uint32_t reg)
{
  *regsel = reg;
  return *iowin;
}

static inline void
ioapic_write (const uint32_t reg, const uint32_t val)
{
  *regsel = reg;
  *iowin = val;
}

void
ioapic_irq_redir (uint8_t irq, uint8_t vector)
{
  uint8_t lapic_id = lapic_get_id ();

  ioapic_write (IOAPIC_IRQ_REDTBL (irq) + 1, (uint32_t)lapic_id >> 24);
  ioapic_write (IOAPIC_IRQ_REDTBL (irq), (uint32_t)vector);
}

void
ioapic_init ()
{
  // ask ACPI for the I/O APIC physical MMIO address
  paddr_t pa = acpi_get_ioapic ();

  // map it in the kernel's page table
  vmm_map ((void *)vmm_get_pgtable (), PA_TO_HHDM (pa), pa, VMM_FLAG_NOCACHE);

  // save the MMIO registers
  regsel = (volatile uint32_t *)PA_TO_HHDM (pa);
  iowin = (volatile uint32_t *)(PA_TO_HHDM (pa) + 0x10);

  // TEST: register IRQ1 to vector 0x21 on the BSP core
  // on real HW x2APIC might be on but I don't want to
  // deal with this now
  ioapic_irq_redir (1, 0x21);
}

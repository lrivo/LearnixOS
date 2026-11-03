#include "lapic.h"
#include "amd64.h"
#include "arch/amd64/hpet.h"
#include <learnix/arch/memlayout.h>
#include <learnix/interrupts.h>
#include <learnix/lib/kprintf.h>
#include <learnix/mm/vmm.h>
#include <stdint.h>

static paddr_t
get_lapic_base_phys () {
  return rdmsr (APIC_BASE_MSR) & ~0xFFFULL;
}

static inline uint32_t
lapic_read_reg (uint32_t reg) {
  return *((volatile uint32_t *)(LAPIC_VIRT_BASE + reg));
}

static inline void
lapic_write_reg (uint32_t reg, uint32_t value) {
  *((volatile uint32_t *)(LAPIC_VIRT_BASE + reg)) = value;
}

uint8_t
lapic_get_id (void) {
  // Return the 8 most significant bits of the LAPIC_ID MMIO register.
  return (uint8_t)((lapic_read_reg(LAPIC_ID) >> 24) & 0xFF);
}

void
lapic_init ()
{
  /* Map the LAPIC register at a known virtual address.
   * Important: we must map it with VMM_FLAG_NOCACHE to avoid
   * weird behaviours on real hardware. */
  vmm_map (vmm_get_kern_pgtable (), (vaddr_t)LAPIC_VIRT_BASE,
           get_lapic_base_phys (), VMM_FLAG_WRITE | VMM_FLAG_NOCACHE);

  /* Now we need to calibrate the LAPIC, since we don´t know his
   * frequency, agains a known timer source: the HPET.
   * Note: the LAPIC frequency never changes, as it's not correlated
   * to the core's one that can change with P-states. */

  // Enable the LAPIC by setting APIC_BASE_MSR 11th bit
  wrmsr(APIC_BASE_MSR, rdmsr(APIC_BASE_MSR) | (1 << 11));
  // Mask the LAPIC timer, we don´t need interrupts now
  lapic_write_reg (LAPIC_TIMER_LVT, 0x10000UL);
  // Set the Spurious Interrupt Vector register 8th bit
  lapic_write_reg (LAPIC_SVR, 0x1FF);
  // Set the clock divisor to 16
  lapic_write_reg (LAPIC_TIMER_DCR, 0x03UL);

  /* In order to reduce the noise, especially on VMs, we need to
   * repeat the calibration more times and take their average. */
  uint64_t mean = 0;
  for (int i = 0; i < N_CALIBRATION; i++) {
      mean += lapic_ms_to_ticks(10);
  }
  mean /= N_CALIBRATION;

  // Now mean is the number of LAPIC ticks per 10 ms.
  // Divide by 10 to get the ticks per 1 ms.
  uint64_t tick_per_ms = mean / 10;

  /* Enable the LAPIC timer in periodic mode on vector 32 and
   * clear the MASK bit to enable interrupts firing. */
  lapic_write_reg(LAPIC_TIMER_LVT, 0x20 | 0x20000UL);
  /* Configure the LAPIC to fire every millisecond. */
  lapic_write_reg(LAPIC_TIMER_ICR, tick_per_ms);
}

uint64_t
lapic_ms_to_ticks(size_t ms) {
    /* the HPET expresses ticks in femtoseconds, we need to convert ms
     * into femtoseconds and compute how many HPET ticks we should wait. */
    uint64_t ticks_wait = (ms * FSEC_PER_MS) / hpet_get_clk_period();

    /* start the HPET and initialize the LAPIC at his maximum
     * counter value immediately after. */
    hpet_start(0);
    lapic_write_reg(LAPIC_TIMER_ICR, -1);

    // poll the HPET main counter untill our wanted ms have passed
    while (hpet_get_main_counter() < ticks_wait)
        ;

    // lapic_delta is now the number of LAPIC ticks in N milliseconds
    uint64_t lapic_delta = 0xFFFFFFFF - lapic_read_reg(LAPIC_TIMER_CCR);
    return lapic_delta;
}

// TODO: this should be moved in a general amd64/interrupt.c
// that can fallback to the legacy PIC if the APIC is not found
void
arch_interrupts_eoi (size_t vector)
{
  (void)vector;
  lapic_write_reg (LAPIC_EOI, 0);
}

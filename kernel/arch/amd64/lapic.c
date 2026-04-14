#include "amd64.h"
#include "lapic.h"
#include <learnix/arch/interrupts.h>
#include <learnix/lib/kprintf.h>
#include <learnix/mm/memlayout.h>
#include <learnix/mm/vmm.h>

static physaddr_t 
get_lapic_base_phys ()
{
  return rdmsr(APIC_BASE_MSR) & ~0xFFFULL; 
} 

static inline uint32_t
lapic_read_reg(uint32_t reg)
{
  return *((volatile uint32_t*)(LAPIC_VIRT_BASE + reg));
}

static inline void
lapic_write_reg(uint32_t reg, uint32_t value)
{
  *((volatile uint32_t*)(LAPIC_VIRT_BASE + reg)) = value;
}

void
lapic_init ()
{
  /* Map the LAPIC register at a known virtual address, because MMIO
     is not covered by HHDM. */ 
  vmm_map((void*)vmm_get_pgtable(), LAPIC_VIRT_BASE, get_lapic_base_phys(), VMM_FLAG_NOCACHE); 

  // Enable the LAPIC by setting APIC_BASE_MSR 11th bit
  wrmsr(APIC_BASE_MSR, rdmsr(APIC_BASE_MSR) | (1 << 11));
  
  // Set the Spurious Interrupt Vector register 8th bit  
  lapic_write_reg(LAPIC_SVR, 0x1FF);
  
  // Mask the LAPIC timer before configuring it 
  lapic_write_reg(LAPIC_TIMER_LVT, 0x10000);  
  
  lapic_write_reg(LAPIC_TIMER_DCR, 0x3); 
  lapic_write_reg(LAPIC_TIMER_ICR, 0xFFFFFF);
 
  // Enable the LAPIC timer in periodic mode on vector 32
  lapic_write_reg(LAPIC_TIMER_LVT, 0x20 | 0x20000);
}

void
arch_interrupts_eoi (size_t vector)
{
  lapic_write_reg(LAPIC_EOI, 0);
}

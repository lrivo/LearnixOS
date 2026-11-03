#include <learnix/acpi.h>
#include <learnix/lib/kpanic.h>
#include <learnix/mm/vmm.h>
#include "hpet.h"

static struct hpet_ctx ctx;

void
hpet_init() {
    /* Map the HPET MMIO register at a known virtual address. */
    vmm_map(vmm_get_kern_pgtable(), (vaddr_t)HPET_VIRT_BASE,
        acpi_get_hpet(), VMM_FLAG_WRITE | VMM_FLAG_NOCACHE);

    // Read the General Capabilities and ID MMIO register.
    uint64_t gcid = hpet_read_reg(HPET_REG_GCID);

    // Save the number of available timer. Used to bound-check the macros.
    ctx.timers_cnt = (uint8_t)((gcid >> 8) & 0x1F);

    // Make sure the period (in femtoseconds) is <= 100 nanoseconds as per specification.
    ctx.clk_period = (gcid >> 32) & CLK_PERIOD_MASK;
    if (ctx.clk_period == 0 || ctx.clk_period > CLK_PERIOD_MAX) {
        kpanic("hpet_init: invalid period\n");
    }

    // Compute the frequency using 64-bit arithmetic
    uint64_t freq = FSEC_PER_SEC / (uint64_t)ctx.clk_period;
    ctx.frequency_hz = (uint32_t)freq;
}

void
hpet_start(uint64_t mc_init) {
    // Disable the main counter entirely
    hpet_write_reg(HPET_REG_GC, 0x00ULL);
    // Set the main counter's initial value
    hpet_write_reg(HPET_REG_MC, mc_init);
    // Set the ENABLE_CNF bit and write back
    hpet_write_reg(HPET_REG_GC, 0x1ULL);
}

uint64_t
hpet_get_main_counter() {
    return hpet_read_reg(HPET_REG_MC);
}

uint64_t
hpet_get_clk_period() {
    return (uint64_t)ctx.clk_period;
}

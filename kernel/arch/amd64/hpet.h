#pragma once
#include <learnix/types.h>
#include <learnix/arch/memlayout.h>
#include <stdint.h>

#define HPET_REG_GCID 0x0                       // General Capabilities and ID Register
#define HPET_REG_GC   0x010                     // General Configuration Register
#define HPET_REG_GIS  0x020                     // General Interrupt Status Register
#define HPET_REG_MC   0x0F0                     // Main Counter Value Register
#define HPET_REG_TCC(N) (0x100 + 0x20 * (N))    // Timer N Configuration and Capability Register
#define HPET_REG_CV(N)  (0x108 + 0x20 * (N))    // Timer N Comparator Value Register
#define HPET_REG_IR(N)  (0x110 + 0x20 * (N))    // Timer N FSB Interrupt Route Register

// Femtoseconds conversion constants
#define FSEC_PER_MS     1000000000000ULL         // 10^12 fsec = 1 ms
#define FSEC_PER_SEC	1000000000000000ULL      // 10^15 fsec = 1 second

#define CLK_PERIOD_MAX  0x05F5E100               // 100 nanoseconds converted in femtoseconds
#define CLK_PERIOD_MASK 0xFFFFFFFF

struct hpet_ctx {
    uint8_t timers_cnt;     // number of timers - 1 (N in the macros)
    uint32_t clk_period;    // real HW period in femtoseconds
    uint32_t frequency_hz;  // clk_period converted in Hertz
};

static __attribute__((always_inline)) inline uint64_t
hpet_read_reg(uint64_t reg_offset) {
    return *((volatile uint64_t*)(HPET_VIRT_BASE + reg_offset));
}

static __attribute__((always_inline)) inline void
hpet_write_reg(uint64_t reg_offset, uint64_t value) {
    *((volatile uint64_t*)(HPET_VIRT_BASE + reg_offset)) = value;
}

// === FUNCTIONS === //
void hpet_init();
void hpet_start(uint64_t mc_init);
uint64_t hpet_get_main_counter();
uint64_t hpet_get_clk_period();

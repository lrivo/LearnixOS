/*
 * Local Advanced Interrupt Controller
 * AMD64 Architecture Programmer's Manual Volume 2: System Programming
 */
#pragma once
#include <learnix/arch/types.h>

/*
 *  63      52 51              12 11  10     9   8     7   0
 *   +----------+----------------+----+------+---+-----+---+
 *   | reserved | APIC base phys | AE | EXTD | / | BSC | / |
 *   +----------+----------------+----+------+---+-----+---+
 *
 *   AE (APIC Enable bit): if set (1) the LAPIC is enabled and all
 *                         interrupt types are accepted
 *   BSC (BootStrap Core): if set (1) this core is the BSC, otherwhise an AP
 */
#define APIC_BASE_MSR 0x1B

/* LAPIC MMIO register offsets. */
#define LAPIC_ID  0x20              // APIC ID Register
#define LAPIC_EOI 0xB0              // End-Of-Interrupt Register
#define LAPIC_SVR 0xF0              // Spurious Vector Register
#define LAPIC_TIMER_LVT 0x320       // Local Vector Table Register
#define LAPIC_TIMER_ICR 0x380       // Initial Count Register
#define LAPIC_TIMER_CCR 0x390       // Current Count Register
#define LAPIC_TIMER_DCR 0x3E0       // Divide Configuration Register

#define N_CALIBRATION 5             // How many calibration measures to take

// === FUNCTIONS === //
/* Per-code LAPIC initialization and calibration. */
void lapic_init();

/* Uses the HPET as a timer source to determine the number
 * of LAPIC ticks per N milliseconds. */
uint64_t lapic_ms_to_ticks(size_t ms);

/* Returns the LAPIC ID of the calling CPU core. */
uint8_t lapic_get_id(void);

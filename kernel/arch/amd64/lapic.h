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
#define LAPIC_EOI 0xB0 // End-Of-Interrupt Register
#define LAPIC_SVR 0xF0 // Spurious Vector Register
#define LAPIC_TIMER_LVT 0x320
#define LAPIC_TIMER_ICR 0x380
#define LAPIC_TIMER_DCR 0xE0

/* Returns the LAPIC ID of the active core. */
uint8_t lapic_get_id (void);

void lapic_init ();

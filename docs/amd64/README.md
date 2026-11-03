# AMD64

Architecture-specific notes for Learnix's x86-64 backend. These pages complement the shared architecture interface and boot overview in the [architecture guide](../../kernel/arch/README.md).

## Table of contents

- [Exceptions and interrupts](exceptions_interrupts.md) — GDT and IDT setup, exception/interrupt entry and dispatch, and early CPU initialization.
- [HPET](HPET.md) — the high-precision timer and its use in LAPIC timer calibration.
- [LAPIC](LAPIC.md) — local APIC MMIO setup, timer calibration against the HPET, and periodic scheduler interrupts.
- [x86-64 Paging](Paging.md) — four-level address translation, PTE permissions, and the CR3/VMM interface.
- [RDSEED](RDSEED.md) — the AMD64 seed instruction, retry behavior, byte interface, and CSPRNG seeding path.

## Resouces
- [AMD64 Architecture Programmer’s Manual Volumes 1–5](https://docs.amd.com/v/u/en-US/40332_4.11_APM_Vol1-5)
  - Volume 2 is the most helpful for hardware programming

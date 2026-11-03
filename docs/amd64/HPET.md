# HPET: High Precision Event Timer

The HPET is an x86-64 platform timer that lives outside the CPU, on the
chipset. It gives the kernel a stable, high-resolution wall clock that does
not depend on the CPU's own clock, which makes it the natural reference for
calibrating things that do.

Code: [`arch/amd64/hpet.h`](../kernel/arch/amd64/hpet.h) (registers + inline
MMIO accessors), [`arch/amd64/hpet.c`](../kernel/arch/amd64/hpet.c)
(initialization and counter access).

## How it works

The HPET exposes a **main counter**: a free-running (or software-started)
integer counter that the hardware increments once every *clock period*. The
period is a property of the timer block, not of the CPU: the firmware
advertises it in **femtoseconds** (10^-15 s) in the General Capabilities and
ID register. The specification caps it at 100 ns, i.e. 100 000 000 fs, which
means any conforming HPET ticks at least 10 MHz.

Everything else derives from that one number. With the period `P` in
femtoseconds, the frequency is simply

```c
frequency_hz = FSEC_PER_SEC / P;   /* 10^15 / P */
```

and a desired wall-clock interval converts to HPET ticks the same way:

```c
ticks = (milliseconds * FSEC_PER_MS) / P;
```

Note that the main counter only counts *ticks*, not seconds; the conversion
constants in `hpet.h` (`FSEC_PER_MS`, `FSEC_PER_SEC`) exist because the
hardware speaks femtoseconds and the rest of the kernel speaks milliseconds
or hertz.

`hpet_init()` also records the number of timers available (bits 8..12 of the
capabilities register, stored as `timers_cnt`, so timer `N` runs from `0` to
`timers_cnt`). Each timer has its own comparator and interrupt route; Learnix
currently uses only the main counter and leaves the compare timers unused.

Two details matter for correctness:

- The period is validated against `CLK_PERIOD_MAX` (100 ns). A zero or
  out-of-spec period means the capability block was read wrong or the
  hardware is broken, so `hpet_init()` panics instead of dividing by it.
- The capabilities register reports whether the main counter is 32 or 64 bits
  wide (bit 13), and Learnix always reads the full 64-bit register. On this
  platform the counter is 64-bit, so it does not wrap in any realistic uptime.

## Programming it

The HPET is a set of **memory-mapped registers**: the physical base address is
not fixed, it comes from ACPI (the HPET description table, parsed by
`hpet_parse()` in [`acpi/acpi.c`](../kernel/acpi/acpi.c) and returned by
`acpi_get_hpet()`). `hpet_init()` maps one page of it into the kernel page
table at `HPET_VIRT_BASE` with `VMM_FLAG_WRITE | VMM_FLAG_NOCACHE` - uncached
is required, otherwise the CPU may serve stale values out of its cache instead
of reading the counter.

All access goes through two accessors, since every register is 64-bit and
must be touched as `volatile`:

```c
uint64_t gcid = hpet_read_reg(HPET_REG_GCID);
hpet_write_reg(HPET_REG_GC, 0x1);
```

The registers Learnix uses (offsets from `HPET_VIRT_BASE`):

| Offset | Register | Meaning |
|---|---|---|
| `0x000` | `HPET_REG_GCID` | General Capabilities and ID: timers count, counter size, clock period (bits 32..63) |
| `0x010` | `HPET_REG_GC` | General Configuration: bit 0 `ENABLE_CNF` starts/stops the main counter |
| `0x020` | `HPET_REG_GIS` | General Interrupt Status: which timers fired |
| `0x0F0` | `HPET_REG_MC` | Main Counter Value: the counter itself |
| `0x100 + 0x20*N` | `HPET_REG_TCC(N)` | Timer `N` configuration and capabilities |
| `0x108 + 0x20*N` | `HPET_REG_CV(N)` | Timer `N` comparator value |
| `0x110 + 0x20*N` | `HPET_REG_IR(N)` | Timer `N` FSB interrupt route |

Starting the counter is a three-step sequence (`hpet_start()`): disable the
block by clearing `HPET_REG_GC`, write the desired initial value into
`HPET_REG_MC`, then set `ENABLE_CNF` (`0x1`) to let it run. Disabling first
means the initial value is written into a stopped counter rather than one
that is already advancing.

Reading the elapsed time is then just `hpet_get_main_counter()` - there is no
interrupt involved, it is a polled counter.

## Why we need it

The main consumer is **LAPIC timer calibration**
([`arch/amd64/lapic.c`](../kernel/arch/amd64/lapic.c)). The LAPIC's timer is a
decrementing counter driven by a fixed LAPIC clock, and - unlike the HPET -
nothing tells the kernel what that clock is. It is not correlated with the
core clock either (which changes with P-states), so it cannot be derived from
CPUID or the TSC frequency; the only reliable option is to *measure* it
against a timer whose frequency is known.

That is exactly the HPET: it advertises its period, so the kernel knows its
frequency in advance. The calibration is a simple interval measurement:

```c
uint64_t ticks_wait = (ms * FSEC_PER_MS) / hpet_get_clk_period();
hpet_start(0);
lapic_write_reg(LAPIC_TIMER_ICR, -1);      /* LAPIC at max, counting down */

while (hpet_get_main_counter() < ticks_wait)
    ;

uint64_t lapic_delta = 0xFFFFFFFF - lapic_read_reg(LAPIC_TIMER_CCR);
```

Both counters start at (nearly) the same instant; when the HPET has advanced
by the number of ticks that correspond to the requested interval, the LAPIC's
current-count register tells how many LAPIC ticks elapsed. `lapic_init()`
repeats this measurement `N_CALIBRATION` times and averages it, because a
single sample on a VM is noisy. The result is converted to ticks per
millisecond and programmed into the LAPIC's initial-count register, which is
what makes the periodic timer interrupt - and therefore preemption - run at
the intended rate. `arch_stage_2()` in
[`arch/amd64/cpu.c`](../kernel/arch/amd64/cpu.c) calls `hpet_init()` before
`lapic_init()`, so the HPET is already mapped and its period known by then.

The same reasoning applies to any other clock that cannot report its own
frequency: use the HPET as the reference, measure, then program the other
timer. The HPET is a measurement standard, not the thing the kernel runs on.

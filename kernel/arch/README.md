# arch/ - Architecture Support

Everything hardware-specific lives here. Today there is a single backend,
[`amd64/`](amd64/), but the design goal is that the *rest* of the kernel never
knows (or cares) which one is compiled in. That contract - the `arch_xxx()`
functions and headers - is what makes Learnix portable.

## The `arch_*()` contract

The architecture-independent kernel (in `mm/`, `proc/`, `sched/`, ...) calls a
well-defined set of `arch_*()` functions and reads a few arch headers. When
porting to a new architecture you copy `arch/amd64/include` and reimplement
the pieces below. Two headers matter most:

- `<learnix/arch/process.h>` - defines `arch_trap_frame` (the raw CPU-saved
  registers) and `arch_proc_context`, pulled into the generic `struct process`.
- `<learnix/arch/interrupts.h>` - defines `intr_trap_frame` (the full frame
  built by ISR stubs/syscall entry) and the `ARG0../NUM/RET` syscall macros.

See [`docs/include_rules.md`](../../docs/include_rules.md) for the include
layout and a full worked example.

What the arch must provide (sampling):

| Generic need              | amd64 implementation                     |
|---------------------------|------------------------------------------|
| bring up the CPU          | `arch_stage_1()`, `arch_stage_2()`       |
| memory translation        | `arch_pg_map/unmap/va_to_pa` (`paging`)  |
| enable/disable interrupts | `arch_interrupts_*` + LAPIC timer        |
| context switch            | `arch_context_switch()` (`process.c`)    |
| process first-run/exec    | `arch_proc_init()/arch_proc_exec()`      |
| true randomness           | `arch_rand_bytes()` (via `rdseed`)       |

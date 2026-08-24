---
name: explain-boot
description: Use when explaining the LearnixOS boot-up sequence, kernel main, init process (PID 1), idle process (PID 0), multi-stage CPU setup, or how the kernel hands to the scheduler. Source of truth is kernel/main.c (kmain) and docs/README.md.
---

# Explaining the LearnixOS Boot Sequence

In LearnixOS, everything starts at `kmain()` in `kernel/main.c`. The boot
sequence is the single best map of the whole kernel, so explaining it well
teaches almost the entire architecture.

## Annotated boot flow
1. **Limine requests** - `main.c` declares the bootloader requests (base
   revision, framebuffer, memory map, HHDM offset, executable addresses) in a
   special `.limine_requests` section. `kmain()` reads the responses and saves:
   `hhdm_offset`, `kernel_virt_base`, `kernel_phys_base`.
2. **`serial_init()` + `console_init(framebuffer)`** - bring up ring-0 text
   output. `_putchar` (defined here) sends to *both* serial and framebuffer.
3. **`tty_init(console_putchar)`** - the TTY wraps console output so
   userspace can `write` to `stdin`/`stdout` (later installed as fds 0/1).
4. **`vmm_init()`** - saves Limine's kernel page table root (shared by all
   processes; see `explain-memory`).
5. **`arch_stage_1()`** - essential CPU setup: GDT, IDT, exception handlers
   (see `kernel/arch/amd64/gdt.c`, `idt.c`).
6. **`rand_init()`** - seeds the ChaCha20 CSPRNG from the hardware RNG.
7. **`acpi_init()`** - ACPI parsing for hardware/memory discovery.
8. **`pmm_init(memmap)`** - builds the physical frame allocator.
9. **`kmalloc_init(KMALLOC_START, PGSIZE)`** - kernel heap.
10. **`arch_stage_2()`** - post-ACPI CPU setup: points the syscall LSTAR MSR at
    `syscall_entry` (see `explain-syscalls`).
11. **`proc_init()`** - creates the idle process (PID 0), the runqueue sentinel.
12. **`sched_init()`** - initializes the configured scheduler.
13. **`ps2kb_init()`** - PS/2 keyboard.
14. **Spawn init (PID 1)** - `proc_create()`; file-descriptors 0 and 1 point
    at `tty0` (read-only / write-only `file_ops`); `limine_module_get("/boot/init")`
    → `elf_load()` → `arch_proc_init()`; `sched_enqueue(init)`.
15. **`kidle()`** - drops into the idle spinner; the scheduler now runs
    everything.

## Key concepts
- **IDLE = PID 0**, **INIT = PID 1**. Both live at `proc_init`/`proc_create` in
  `kernel/proc/process.c`. They cannot die (`sys_exit` panics if pid <= 1).
- **stage_1 vs stage_2**: some CPU setup must happen after ACPI (which may
  remap memory); hence the two `arch_stage_*` calls.
- **HHDM**: Higher-Half Direct Mapping - Limine maps physical RAM high so the
  kernel access it via `P2V`/`V2P`.

## Answering
Follow the steps in `docs/README.md`'s "thread to follow", then read
`kernel/main.c` to confirm each call and argument. Explain the *ordering
reasons* (e.g. why kmalloc can only come after pmm, why stage_2 after ACPI).
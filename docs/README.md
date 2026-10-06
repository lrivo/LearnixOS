# Learnix Kernel's Documentation
This documentation describes the internal architecture of the kernel. It is
written to be useful both as a reference for the developer and as a learning
resource for OS students approaching kernel development for the first time.

## Overview
The Learnix kernel is a UNIX-like monolithic kernel for the x86-64 architecture
(at least for now) completely written in C and NASM assembly.

It uses the **Limine bootloader**, which conveniently loads the kernel in long
mode with a **Higher-Half Direct Mapping (HHDM)** so the code can access the
physical RAM through a simple translation.

The core kernel code is already written as **architecture independent**,
meaning Learnix can be ported to other hardware as well (ARMv8 and RISC-V are
next). The contract that makes this work - the `arch_xxx()` functions and
headers - is described under [arch/](../kernel/arch/README.md).

Learnix is designed to swap and test different components at compile time; for
now the swappable set is:
- the physical memory manager (`-Dpmm=bitmap|freelist`)
- the kernel heap (`-Dkmalloc=freelist|watermark`)
- the scheduler (`-Dsched=rr|mlfq|lottery`)

See `meson.options` for the compile-time knobs.

## A thread to follow on first read
There is no better overview of the whole system than the **boot sequence**, so
the docs are arranged to walk it top to bottom. `kmain()` in
`kernel/main.c` shows the real order; this is the annotated version:

1. **Save Limine request info** - HHDM offset, the kernel's executable bases.
2. **Consoles** - `serial_init()` and `console_init(framebuffer)` bring up the
   ring-0 text output (`_putchar` → serial *and* framebuffer). The **TTY**
   wraps this so userspace can `write` to `stdin`/`stdout`.
3. **`vmm_init()`** - saves Limine's kernel page table root
   (see [mm/](../kernel/mm/README.md)).
4. **`arch_stage_1()`** - essential CPU setup: GDT, IDT, exceptions.
5. **`rand_init()`** - seeds the ChaCha20 CSPRNG from hardware true randomness
   (rdseed) (see [security/](../kernel/security/README.md)).
6. **`acpi_init()`** - parses ACPI to discover hardware/memory regions.
7. **`pmm_init()`** - builds the physical frame allocator from Limine's
   memory map.
8. **`kmalloc_init()`** - initializes the kernel heap.
9. **`arch_stage_2()`** - post-ACPI CPU setup (puts `syscall_entry` into the
   LSTAR MSR).
10. **`proc_init()`** - spawns the idle process (PID 0).
11. **`sched_init()`** - initializes the chosen scheduler.
12. **`ps2kb_init()`** - the PS/2 keyboard driver.
13. **Spawn init (PID 1)** - `proc_create()`; installs the TTY as fds 0 and 1;
    ELF-loads `/boot/init`; links init's user stack; enqueues it.
14. **`kidle()`** - the kernel drops into the idle spinner, letting the
    scheduler run everything from here.

From that point, control flows through the scheduler and the syscall layer -
the two subsystems opened below.

## Subsystems
Detailed, code-walking documentation for each part of the kernel:

- [arch/ - CPU, GDT, IDT, paging, context switch, syscall trap](../kernel/arch/README.md)
- [mm/ - Memory Management](../kernel/mm/README.md)
- [proc/ - Processes](../kernel/proc/README.md)
- [sched/ - Scheduler](../kernel/sched/README.md)
  - [Round Robin](../kernel/sched/rr/README.md)
  - [Lottery](../kernel/sched/lottery/README.md)
- [sys/ - System Call Layer](../kernel/sys/README.md)
- [fs/ - Files, Inodes and Pipes](../kernel/fs/README.md)
- [lib/ - Kernel Utilities (kprintf, ELF, Limine modules)](../kernel/lib/README.md)
- [security/ - Canaries and the CSPRNG](../kernel/security/README.md)
- [include_rules/ - How headers & arch portability are laid out](include_rules.md)

## References
- [Operating Systems: Three Easy Pieces](https://ostep.org) - Arpaci, Dusseau
- [Understanding the Linux Kernel 3rd Edition](https://www.cs.utexas.edu/~rossbach/cs380p/papers/ulk3.pdf) - Daniel P. Bovet, Marco Cesati, 2005
- [OSDev Wiki](https://wiki.osdev.org)
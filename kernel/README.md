# Kernel

This folder contains the Learnix kernel: a UNIX-like, monolithic x86-64
kernel written largely in **architecture independent C**. Everything below
lives here unless it is device/specific, in which case it lives under
`arch/`.

- `arch/` – hardware specific code (currently only `amd64`). This is where the `arch_*()` functions, GDT/IDT, paging, trap frames, and context switch live.
- `drivers/` – device drivers (keyboard, serial console, ...)
- `include/` – generic, architecture-independent kernel headers under `<learnix/...>`
- `lib/` – libc-like utilities and kernel services (`kprintf`, `memcpy`, ELF loading, `limine_module`)
- `mm/` – architecture independent memory management (PMM, VMM, kmalloc)
- `proc/` – everything about processes (PCBs, fork, exit, wait, ...)
- `sched/` – schedulers (round-robin, lottery)
- `sys/` – system call implementations (read/write/pipe/mmap, ...)
- `fs/` – the file descriptor / inode abstraction behind `sys/`
- `security/` – canaries and the ChaCha20 CSPRNG
- `acpi/` – ACPI parsing (used to discover devices and memory regions)

Entry point: `main.c` defines `kmain()`, the boot sequence that initializes
every subsystem before handing the CPU to the scheduler.

### Notes
Currently the PS/2 keyboard driver (`drivers/input/ps2kb.c`) sits in the
architecture-independent folder but is in fact x86-64 specific (it drives the
8259 PIT/legacy PIC). I am leaving it this way for now,
but it is a known refactor candidate: it probably belongs under
`arch/amd64/drivers/` conceptually, or at least behind an `arch_`-style
indirection. I don't want to guess the right shape until other architectures
become real.

Read the [documentation](../docs/README.md) for the architecture-level
story.

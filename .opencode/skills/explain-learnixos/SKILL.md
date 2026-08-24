---
name: explain-learnixos
description: Use when explaining how the LearnixOS kernel works, answering questions about the codebase, or onboarding a new reader to the project. Provides the authoritative map of subsystems, their source-of-truth docs, key architecture concepts (HHDM, arch_*() portability contract, init/PID 0, syscall trap), and known pitfalls.
---

# Explaining LearnixOS

LearnixOS is the author's didactic x86-64 kernel (Bachelor's thesis) written
in C + NASM assembly. When asked what something *means* or *does*, use this
skill to orient the reader with the accurate mental map, then point into the
right source files.

## Ground rules
1. **Never infer from generic OS knowledge alone.** This kernel makes specific
   design choices that differ from Linux. Always ground answers in the
   source-of-truth docs listed below and the code, not in "how it usually
   works".
2. **Read the subsystem README before the code.** Every major folder has a
   teaching README that explains the *why*. Read the relevant one first, then
   the `.c` files to confirm details.
3. **Anchor terminology.** Use the kernel's own names: `kmain`, `arch_*()`,
   `struct file`/`file_ops`, `intr_trap_frame`, `uvm`, `procs[]`, etc.
4. **Flag known bugs** where relevant (they are marked `BUG`/`FIXME` in code
   and tracked in `todo.md`).

## Source of truth docs (read these first)
- `docs/README.md` - overall architecture + annotated boot sequence.
- `docs/include_rules.md` - the include layout and arch-portability contract.
- Per-subsystem READMEs under `kernel/*/README.md`:
  - `arch/` - CPU, GDT/IDT, paging, context switch, syscall trap.
  - `mm/` - PMM → VMM → kmalloc layering.
  - `proc/` - process lifecycle, PCB, fork/exit/wait.
  - `sched/` (+ `rr/`, `lottery/`) - scheduler mechanism vs policy.
  - `sys/` - the system call layer.
  - `fs/` - file descriptors, inodes, pipes.
  - `lib/` - kprintf, ELF loader, Limine module lookup.
  - `security/` - canaries + ChaCha20 CSPRNG.

## Subsystem map
| Folder | Role | Key entry point |
|--------|------|-----------------|
| `arch/amd64` | x86-64 specifics; `arch_*()` contract | `gdt.c`, `idt.c`, `process.c`, `syscall.asm` |
| `mm/` | memory management | `pmm_*.c`, `vmm.c`, `uvm.c`, `mmap.c` |
| `proc/` | processes | `process.c`, `fork.c`, `exit.c` |
| `sched/` | scheduler selection | `sched.c`, `rr/`, `lottery/` |
| `sys/` | syscall handlers + dispatcher | `syscall.c`, `pipe.c`, `tty.c` |
| `fs/` | fd & inode abstraction | `file.c`, `inode.c` |
| `lib/` | utilities | `elf.c`, `kprintf.c`, `limine_module.c` |
| `security/` | CSPRNG + canaries | `canary.c`, `chacha20.c` |

## Core mental models worth repeating
- **Portability contract**: generic kernel calls `arch_*()` and reads
  `<learnix/arch/...>`. New HW = copy + reimplement. See `docs/include_rules.md`.
- **Boot flow**: `kmain()` in `kernel/main.c` runs consoles → vmm → GDT/IDT →
  CSPRNG → ACPI → pmm → kmalloc → stage_2 → idle proc → scheduler → ps2kb →
  init (PID 1) → `kidle()`. Documented top-to-bottom in `docs/README.md`.
- **Syscall journey**: userspace `syscall` → `syscall.asm` trampoline → pinned
  `intr_trap_frame` → `syscall_dispatcher()` vtable → handler (`sys_read`, ...)
  → `kernel_exit` → `iretq`.
- **init = PID 1** (first user process), **idle = PID 0** (runqueue sentinel).

## Answering a specific question
1. Identify which subsystem the question is about.
2. Read that subsystem's README (and the umbrella `docs/README.md`) as context.
3. Read the relevant `.c`/`.h` to ground the answer in real behavior.
4. Explain in teaching style (why + how), referencing both `path:line`.

## If you need to inspect
Use the Read/Glob/Grep tools on the working tree. Everything kernel lives
under `/kernel/`; headers for generic code under `kernel/include/learnix/`,
arch headers under `kernel/arch/amd64/include/learnix/arch/`.

Use the focused skills when the question targets a single subsystem:
- `explain-boot` - boot sequence and init.
- `explain-syscalls` - the system call layer.
- `explain-memory` - PMM/VMM/kmalloc.
- `explain-scheduling` (plus `explain-processes`) - scheduler and process.
# LearnixOS — Agent Instructions

This is a didactic x86-64 kernel (Bachelor's thesis) written in C + NASM
assembly. When working or answering questions about it, please follow these
conventions.

## Before answering anything code-related
Read the project documentation first; it is the authoritative source and is
written specifically to teach this exact codebase:

- `docs/README.md` — architecture overview + annotated boot sequence
- `docs/include_rules.md` — include layout and the arch-portability contract
- `kernel/*/README.md` — a teaching README per major subsystem
- `todo.md` — known bugs / refactor ideas / roadmap

Use the `explain-*` skills for in-depth subsystem explanations.

## Layout (TL;DR)
| Path | What it is |
|------|------------|
| `kernel/main.c` | `kmain()` — the boot sequence / entry point |
| `kernel/arch/amd64/` | x86-64 specifics: GDT, IDT, paging, process, syscall.asm |
| `kernel/mm/` | physical (PMM), virtual (VMM), kmalloc, mmap |
| `kernel/proc/` | process PCB, fork, exit, wait, execve |
| `kernel/sched/` | schedulers: round-robin, lottery |
| `kernel/sys/` | system call dispatcher + handlers |
| `kernel/fs/` | `struct file`/`file_ops` vtable, inode, pipe |
| `kernel/lib/` | kprintf, ELF loader, Limine module lookup |
| `kernel/security/` | ChaCha20 CSPRNG + stack canaries |
| `liblearnix/` | the userspace libc |
| `userspace/` | user programs: init, shell, echo, test, brainfuck |

## Conventions
- Don't assume generic OS behavior; ground answers in the code and these docs
  (this kernel deliberately differs from Linux).
- Architecture-independent code lives under `kernel/`, arch-specific under
  `kernel/arch/amd64/`. `docs/include_rules.md` explains the header split.
- Respect the existing teaching-doc voice: why + how, with links to code.
- Don't add code comments unless asked (project convention).

## Working On
VMA (Virtual Memory Area) implementation (sorted singly linked list):
- [x] basic functions (alloc, free, insert, remove, search)
- [ ] mmap/munmap rewrite
    - best first, less moving parts than fork/exec
    - test demand paging (map a VMA not present, then access such memory and #PF)
- [ ] fork/exec rewrite 

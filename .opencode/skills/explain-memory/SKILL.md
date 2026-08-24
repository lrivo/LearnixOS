---
name: explain-memory
description: Use when explaining memory management in LearnixOS - physical frame allocator (PMM), virtual memory manager (VMM), kernel heap (kmalloc), mmap/munmap, page tables, or the memory layout macros. Source of truth is kernel/mm/README.md and the mm/ directory.
---

# Explaining LearnixOS Memory Management

Memory management in LearnixOS is a three-layer stack (each builds on the
one below it), documented in `kernel/mm/README.md`:

```
┌──────────────┐  kmalloc() - kernel heap
├──────────────┤  vmm()     - virtual addresses & page tables
├──────────────┤  pmm()     - physical 4KB frames
└──────────────┘
```

## 1. PMM - Physical Memory Manager
- Reads Limine's memory map and tracks every physical 4KB frame.
- Required by Learnix to be **refcount-aware**: it knows when a physical page
  is shared between multiple mappings.
- Swappable implementations live in `kernel/mm/pmm_*.c` (bitmap, list),
  selected at build time (`-Dpmm=bitmap|list`).

## 2. VMM (Virtual Memory Manager)
- Manages the **page table** - the OS's core address abstraction.
- Architecture-agnostic: it delegates the real walk to `arch_pg_map`,
  `arch_pg_unmap`, `arch_va_to_pa` (defined by each arch; amd64's is in
  `kernel/arch/amd64/paging.c`). See the `arch_*()` contract in
  `docs/include_rules.md`.
- Each process carries its own page table root (`struct process.pgtable`) and
  an `uvm` (user VM) that inherits the kernel's mappings and adds the
  process's own.
- `mmap.c` implements the mmap/munmap syscalls.

## 3. kmalloc (Kernel Heap)
- The dynamic allocator for kernel-internal structs like PCBs.
- Provides **virtual continuity**, not physical (see the README note).

## Key interactions to call out
- `fork()`/`execve()` copy/destroy a process's `uvm`; the copy walks the
  parent's x86-64 page tables level by level
  (`arch_uvm_copy_or_destroy`, in `arch/amd64/process.c`).
- `elf_load()` calls into `vmm_map`/`pmm_alloc` to materialize a program's
  sections (see `explain-syscalls` / `lib/` README).
- The **HHDM** translation (see `explain-boot`) is what lets a physical page
  address be treated as a virtual pointer via `P2V`.

## When explaining
1. Locate which of the three layers the question touches.
2. Open `kernel/mm/README.md` for the mental model.
3. For VMM behavior, additionally consult `arch/amd64/paging.c` (`pgdirwalk`
  is the heart) and `arch/amd64/process.c` (copy/teardown).
4. Mention the swappable-compile-time aspect where relevant.

Known wart: `mm/mmap.c` is flagged in `todo.md` as "very very bad" - good to
note when explaining mmap internals.
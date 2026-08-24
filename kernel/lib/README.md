# lib/ - Kernel Utilities

This folder is the kernel's "stdlib": helpers that are not a subsystem in
their own right but are used by many others. Think of it as the libc of
Learnix (plus a few kernel-specific services that did not earn their own
folder).

- **`kprintf.c`** - the kernel's printf/print routine. Every subsystem uses it
  to log; `_putchar` in `main.c` routes its output to both the serial console
  and the framebuffer console.
- **`string.c`** - `memcpy`, `strlen`, `strcmp`, ... under `<learnix/lib/string.h>`.
- **`debug.c`** - assertion and crash debugging helpers.
- **`rand.h`** - the thin interface over the CSPRNG implemented in
  `security/chacha20.c`.
- **`elf.c`** - loads a userspace ELF executable into a fresh page table.
- **`limine_module.c`** - looks up a bootloader-provided module (a raw file)
  by path; this is the heart of the kernel's "ramfs".

## ELF loader (`elf.c`)

`elf_load(hdr, pgdir)` turns a raw ELF image (from the Limine module list)
into a runnable address space:

1. **Validates** (`elf_validate`): checks the ELF magic, that it is an
   executable (`ET_EXEC`) for this machine (`EM_AMD64`), returning an errno
   otherwise.
2. **Walks the program header table** (`e_phoff` → `e_phnum`). For each
   `PT_LOAD` section it maps the segment's virtual address onto freshly
   allocated physical pages, copying in the section bytes across page
   boundaries; for `PT_STACK` it maps a fixed `USR_STACK` region.
3. Enforces a **W^X policy**: a section that would end up both writable and
   executable is rejected up front.

> 🐞 **Known issues (tracked in `todo.md`):** `elf_load` implicitly assumes
> every section is ~4KB/page-aligned, and it does not check whether two
> sections land in the same page. Both are ripe for a cleanup.

## Limine module lookup (`limine_module.c`)

Learnix's "filesystem" is currently the bootloader's **ramfs**: Limine hands
the kernel a list of modules (`struct limine_file`), each with a path (e.g.
`/boot/init`), an address, and a size. `limine_module_get(path)` walks that
list and returns the module or `NULL`. Both the inode/ramfs (in `fs/`) and the
ELF loader build on this - `sys_open()` and `sys_execve()` resolve their
pathnames through it.

## How it hangs together
A clean little upward story: `limine_module_get("/boot/init")` supplies the
bytes that `elf_load` turns into the first user program, and `inode.c` uses
the same lookup to back `open()`. So the bootloader's ramfs is the ultimate
source of every file *and* every executable the early kernel sees.

See also: [`fs/`](../fs/README.md) for the inode/ramfs layer, and
[`sys/`](../sys/README.md) for the syscalls that drive it.
# Learnix Include Rules

When compiling we set the following include paths (relative to `/kernel`):
- `-I include/` that resolves to `#include <learnix>` for generic kernel code
- `-I arch/$ARCH/include/` that resolves to `#include <learnix/arch>` for architecture specific definitions

## include/learnix - Generic kernel interfaces
- Architecture-independent structs, typedefs and function declarations.
- May pull arch-specific stuff via `#include <arch/X.h>`.

Let's take `proc.h`, which defines the Process Control Block, as an example:

```c
#pragma once
#include <learnix/types.h>
#include <learnix/arch/proc.h>   // this defines arch_trap_frame

struct process
{
  pid_t pid;
  void *kstack;
  struct arch_trap_frame *tf;
}

void arch_switch_to(struct process *prev, struct process *next);
```

The only thing which is architecture specific in a PCB is obviously the `arch_trap_frame` which refers
to the kernel stack memory layout needed for context switch.
For example x86_64 will push `SS, RSP, RFLAGS, CS, RIP` and expects to find them before calling `iretq`
to return executing in ring 3.

The `arch_switch_to()`, which does context switching, will be implemented under a `process.c` of the
target architecture.

## arch/XXX/include/learnix/arch - Architecture Specific interfaces
- Architecture-dependent structs, typedefs and `arch_xxx()` functions.
- When porting to a new architecture just copy the `arch/amd64/include` folder and start changing the definitions.

For example x86_64 has this stack frame that is automatically pulled by the generic `struct process`: 

```c
#pragma once
#include <stdint.h>

struct arch_trap_frame 
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));
```

ARM or RISC-V will have to change this and change actual the `arch_xxx()` functions.

## include/uapi/learnix - The kernel–userspace ABI

The kernel and liblearnix are two separate builds, but they talk to each other:
syscall numbers, errno constants, `mmap` flags and the userspace-visible structs
(`sched_stats`, `pmm_stats`) are a contract both sides must agree on. Before
this directory existed the same defines were hand-copied in
`kernel/include/learnix/syscall.h` and `liblearnix/include/sys/syscall.h`, and
any edit could silently desync the two sides and break the ABI.

This is the same problem Linux solved in 2012 with the `include/uapi/` split
(before that, kernel headers carried `#ifdef __KERNEL__` blocks), so the layout
here follows theirs. Everything userspace can see lives **once** in
`include/uapi/learnix/` (e.g. `syscall.h` defines the `SYS_*` numbers that both
`kernel/sys/syscall.c` and `liblearnix/liblearnix.c` dispatch on), and the two
builds resolve the *same files* through different include paths:

- the kernel spells them `<uapi/learnix/syscall.h>` (Meson `uapi_hdrs_dep`,
  wired like `freestnd_hdrs_dep`). The kernel-side headers in
  `kernel/include/learnix/` are thin wrappers that include the uapi version and
  add kernel-only stuff on top — exactly like Linux's `include/linux/futex.h`
  starting with `#include <uapi/linux/futex.h>`. See `syscall.h`,
  `sched_types.h` and `mm/pmm_stats.h` for the pattern;
- userspace spells them `<learnix/syscall.h>` (`uapi_user_hdrs_dep`, inherited
  by every program through `liblearnix_dep`). Linux does the same rename when
  `make headers_install` exports headers to `/usr/include` — there the `uapi/`
  prefix is stripped; here the rename is just a second include path because
  kernel and libc are built in the same tree.

Why not share libc-named headers (`errno.h`, `sys/mman.h`, ...) directly?
Linux never exports those names: they belong to the C library. The uapi headers
carry only numbers, flags and structs — zero function prototypes. Prototypes
are liblearnix's job: `sys/syscall.h` declares `_do_syscall`, `sys/mman.h`
declares `mmap`/`munmap` (and pulls the flags from `<learnix/mman.h>`, like
glibc's `sys/mman.h` includes the kernel values). `SEEK_*` are the one
duplicate that stays: their values are fixed by POSIX, so Linux and glibc each
define their own too.

Rules for headers in `include/uapi/learnix/`:
- self-contained: only freestnd headers (`stdint.h`) for types, no kernel or
  libc internals, no function prototypes;
- userspace-visible structs live here, not in `kernel/include/learnix/`.

## Pre-refactor notes
Before the include refactor explained above I also had a `kernel/include/learnix/arch` directory that defined all the `arch_xxx()`.
This was ok at the beginning but then I found myself having too many headers for the same stuff so I opted for the include
rules you've just read.

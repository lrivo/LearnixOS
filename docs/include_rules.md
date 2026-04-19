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

## Pre-refactor notes
Before the include refactor explained above I also had a `kernel/include/learnix/arch` directory that defined all the `arch_xxx()`.
This was ok at the beginning but then I found myself having too many headers for the same stuff so I opted for the include
rules you've just read.

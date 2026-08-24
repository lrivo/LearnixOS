---
name: explain-syscalls
description: Use when explaining how system calls cross the user-to-kernel boundary in LearnixOS, the syscall dispatcher, trap frame, arg/RET macros, error/errno conventions, or any individual syscall (read, write, open, pipe, mmap, yield, fork, ...). Source of truth is kernel/sys/syscall.c and arch/amd64/syscall.asm.
---

# Explaining the LearnixOS System Call Layer

In LearnixOS, userspace cannot call kernel functions directly - it must cross
from ring 3 to ring 0 through a CPU-supported syscall gate.

## The full journey (x86-64)
1. **Userspace** executes `syscall <num>` with `rax` = syscall number and up
   to 6 args in the ABI registers:
   - `NUM=rax, ARG0=rdi, ARG1=rsi, ARG2=rdx, ARG3=r10, ARG4=r8, ARG5=r9, RET=rax`
2. The CPU jumps to the handler in the **LSTAR MSR** set by `arch_stage_2()`:
   `kernel/arch/amd64/syscall.asm::syscall_entry`.
   - `swapgs`; saves the user's `rsp` in the process's context slot; pivots the
     stack pointer onto the current process's **kernel stack**
     (`kstack + KSTACK_SIZE`).
   - Pushes a full `intr_trap_frame` (all GP registers + `rip/cs/rflags/rsp/ss`
     + zeroed vector/error slots) then `call syscall_dispatcher` with the frame
     in `rdi`.
   - `jmp kernel_exit`, which eventually `iretq`s back into userspace.
3. **`syscall_dispatcher(tf)`** in `kernel/sys/syscall.c` indexes a 128-slot
   vtable `sys_table[]` by `NUM(tf)` and calls the handler; unknown numbers get
   `RET(tf) = -1`.

## Reading and writing the frame
The macro `NUM/RET/ARG0..ARG5` in
`kernel/arch/amd64/include/learnix/arch/interrupts.h` just read/write named
fields of `struct intr_trap_frame`. So handlers look like plain C:

```c
void sys_read(struct intr_trap_frame *tf) {
  int fd = (int)ARG0(tf);
  void *buf = (void*)ARG1(tf);
  size_t count = (size_t)ARG2(tf);
  ...
  RET(tf) = result;
}
```

## Handling by type of object
Nearly every handler ultimately operates on a `struct file` found via
`arch_cpu_get()->proc->fds[fd]` and calls into its `file_ops` vtable
(`.read/.write/.lseek/.close`). This is why one dispatcher serves TTYs,
inode-backed files, and pipes uniformly - see `explain learnixos fs` / the
`fs/` README.

## Errors
Errors are returned as **negative errno**: `RET(tf) = -EBADF`, etc. Note some
handlers still use raw magic numbers (`-1`, `-2`, `-3`) instead of errno -
tracked as known bugs.

## Registration
When you text a new syscall: (1) give it a `SYS_*` number constant, (2) add a
slot in `sys_table[]`, (3) implement the handler with `tf`. The userspace side
declares prototypes in `liblearnix/include/sys/syscall.h`.

## Non-POSIX extras
`SYS_SET_FG_PROC` and friends are tagged `/* --- non POSIX syscalls --- */` -
Learnix-specific console helpers.

When asked to explain a specific syscall, read its handler in
`kernel/sys/*.c` (or `kernel/proc/*.c` for fork/execve/exit/wait/mmap), trace
it through the `file_ops` vtable, and tie it back to this frame/dispatcher flow.
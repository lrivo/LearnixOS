# sys/ - System Call Layer

User programs must be able to ask the kernel to do privileged things for them:
read and write files, spawn processes, allocate memory. They cannot do any of
this by simply calling a function, because they need to cross the **user space
→ kernel space** boundary, and only the kernel is trusted enough to mess with
hardware and the page tables of other processes.

This folder is where the requests of userspace programs land: the syscall
dispatcher and every syscall implementation (read, write, open, pipe, mmap,
yield, fork, ...).

## How a system call happens (x86-64)

A user program that wants to make a syscall does `syscall <num>`, with the
syscall number in `rax` and up to 6 arguments in the ABI registers:

```
NUM  = rax     ARG0 = rdi     ARG1 = rsi     ARG2 = rdx
ARG3 = r10     ARG4 = r8      ARG5 = r9      RET  = rax
```

The CPU switches to ring 0 and jumps to the handler stored in the **LSTAR MSR**
(`arch/amd64/syscall.asm`). That tiny trampoline:

1. `swapgs` to the kernel's GS base.
2. Saves the user's `rsp` in the PCB and pivots the stack onto **the current
   process's kernel stack** (`kstack + KSTACK_SIZE`).
3. Pushes a full `intr_trap_frame` (all 15 GP registers + the `rip/cs/...`
   segments needed by `iretq`; the vector/error slots are zeroed for syscalls,
   unlike real interrupt frames).
4. `call syscall_dispatcher` (this file), passing the frame as `rdi`.
5. Jumps to the shared `kernel_exit`, which eventually `iretq`s back to the
   userspace process.

## The dispatcher

`syscall_dispatcher(struct intr_trap_frame *tf)` (`sys/syscall.c`) is the
brain of the syscall layer. It is a classic **dispatch table**:

```c
static void (*sys_table[128])(struct intr_trap_frame *tf) = {
  [SYS_READ]   = sys_read,
  [SYS_WRITE]  = sys_write,
  [SYS_MMAP]   = sys_mmap,
  ...
};

void syscall_dispatcher(struct intr_trap_frame *tf) {
  size_t num = NUM(tf);
  if (num < 128 && sys_table[num])
    sys_table[num](tf);
  else
    RET(tf) = -1;   // unknown syscall
}
```

The macros `NUM/RET/ARG0..ARG5` (`arch/amd64/.../interrupts.h`) just read/write
the right fields of the trap frame, so a handler looks like a normal C
function:

```c
void sys_write(struct intr_trap_frame *tf) {
  int fd = (int)ARG0(tf);
  void *buf = (void*)ARG1(tf);
  size_t count = (size_t)ARG2(tf);
  ...
  RET(tf) = f->ops->write(f, buf, count);
}
```

## Error handling & errno

A handler signals failure by returning a **negative errno**, e.g.
`RET(tf) = -EBADF`. POSIX errno constants (`EBADF`, `EINVAL`, ...) are reused
from the userspace headers.

> 🐞 **Known bugs being tracked here:**
> - `sys_read` / `sys_write` dereference the userspace `buf` pointer without
>   any validation (`// BUG: unchecked user controlled pointer`). A malicious
>   program could hand the kernel an arbitrary address.
> - `sys_pipe` returns raw errors (`-1`, via a `bad:` cleanup label) rather
>   than errno.

## The fd abstraction it all hangs on

Every handler ultimately operates on a `struct file` obtained from the
process's `fds[]` table. The *object* behind the descriptor is opaque; all the
kernel knows is its `file_ops` vtable (`.read/.write/.close/.lseek`). That is
how one syscall family serves a TTY, an inode-backed file, and a pipe alike.
The details live in [`fs/`](../fs/README.md).

## Non-POSIX extras
Beyond the classic UNIX syscalls, Learnix adds a few of its own (tagged
`/* --- non POSIX syscalls --- */`), such as `SYS_SET_FG_PROC` to switch the
foreground process of the console, the scheduler stats pair
(`SYS_SCHED_GET_STATS` / `SYS_SCHED_SET_PRIO`), and the PMM stats pair
(`SYS_PMM_STATS_START` / `SYS_PMM_STATS_GET`) used to benchmark the physical
allocator from userspace.

Subsystem links: [fs/](../fs/README.md) · [proc/](../proc/README.md) ·
[mm/](../mm/README.md) · [arch/](../arch/README.md) for the trap-frame
mechanics.
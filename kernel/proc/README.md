# proc/ - Process Subsystem

A process is the **OS's fundamental execution unit**. It is the running
incarnation of a program along with the resources the OS has granted to it: an
address space, a kernel stack, a set of open files, and a place in whatever
run-queue the scheduler maintains.

## The Process Control Block (PCB)

Each process is tracked by a `struct process` (**`include/learnix/process.h`**),
the [Process Control Block](https://en.wikipedia.org/wiki/Process_control_block)
of Learnix. It holds:

- **`pid`** - process identifier, looked up as an index into the global `procs[]` table.
- **`state`** - one of `RUNNING`, `READY`, `WAITING`, `ZOMBIE` (see below).
- **`kstack`** - the *kernel* stack this process uses while executing kernel code. Every process has its own, so a syscall can't clobber another process's kernel control flow.
- **`pgtable`** - the root of this process's page table (its private address space).
- **`tf`** - pointer to the `intr_trap_frame` living on the kernel stack. This is how the CPU "remembers" where a *user-mode* process was suspended so it can resume it.
- **`ctx`** - the architecture-specific register context used for context switching.
- **`parent` / `child_wq`** - support for the `wait()`/`exit()` flow.
- **`fds[]`** - the process's file descriptor table (which files it has open).
- **`priority`, `next`, `prev`** - scheduler bookkeeping.

> The PCB mixes architecture-independent fields with two architecture-specific
> ones (`tf`, `ctx`). The headers keep them separate: `process.h` pulls in
> `<learnix/arch/process.h>`, so porting to another architecture only means
> changing that one header. See `docs/include_rules.md`.

## Lifecycle

Learnix tracks every process in a flat global table `procs[PROCS_LEN]`
(`proc/process.c`). A pid is just an index into it, so lookup is `O(1)`.

```
        ┌──────────┐    proc_create()              ┌───────────┐
        │  CREATED │──────────────────────────────▶│  READY    │
        └──────────┘                               └─────┬─────┘
                                                       sched picks it
                                                          │
                                                          ▼
                                                ┌───────────────┐
                                                │   RUNNING     │
                                                └──┬───────┬────┘
                                      I/O / yield  │       │  quantum expires
                                                   ▼       ─────▶ READY
                                              ┌──────────┐
                                              │  WAITING │
                                              └──────────┘
                                 exit()
                                     │
                                     ▼
                              ┌───────────┐   parent calls
                              │  ZOMBIE   │──── wait() ──▶ pmm/kfree
                              └───────────┘
```

- **proc_init()** - creates the **idle process** (pid 0), a kernel-mode spinner that runs only when nothing else can. It is the *sentinel* of the runqueue.
- **proc_create()** - allocates a zeroed PCB, hands out the next free pid, gives it a fresh kernel stack and a **copy of the kernel's page table** (so it doesn't need an MMU switch on every interrupt). See the long comment in `process.c` for the caveat about late kernel page-table additions.
- **proc_exit() / sys_exit** - the dying process closes its open file descriptors, **reparents its children to PID 1** (init), turns itself into a `ZOMBIE`, wakes its parent's wait queue, and removes itself from the runqueue. It does not free its own PCB: it runs *on its own kernel stack*, so it cannot free that stack from underneath itself. Only when `wait()` is called does `proc_destroy()` reclaim the page table, kernel stack, pid, and PCB.
- **proc_destroy()** - the actual teardown, called by `sys_wait()` once a zombie child has been found.

## Walkthrough: fork()

`fork()` (`proc/fork.c`) is the classic "copy everything" syscall:
1. `proc_create()` a child process.
2. Copy the parent's *userspace* mappings into the child with `uvm_copy()` (**eager style**), giving the child its own private address space.
3. `memcpy(parent->tf, child->tf)`: the child starts where the parent *would* have resumed, so `sys_fork` returns twice - to the parent with the child's pid, and to the child with 0.
4. Duplicate the file descriptor table, bumping each file's refcount.
5. Enqueue the child in the scheduler's runqueue.

## The user/system boundary
This subsystem really stops at bookkeeping. The **ELF loading** that turns a flat file into a runnable address space lives in `mm/` (the `uvm_*` functions) and `lib/elf.c`; `sys_execve()` in `proc/execve.c` orchestrates it. The actual *context swap* between two running processes is an architecture-level operation (`arch_context_switch()`), described in the arch README.

See also: [`sched/`](../sched/README.md) for how READY processes are chosen, and
[`sys/`](../sys/README.md) for how the syscall dispatcher reaches these handlers.

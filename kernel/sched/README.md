# sched/ - Scheduler Subsystem

## Overview
A modern personal computer has one CPU (with multiple physical/logical cores) that must be shared between hundreds of processes.
Obviously a CPU core can only execute one process at any given time and so, to give the **illusion that many processes are running at the same time**we use the scheduler to "time share" the CPU and context switch between them.

### Preemptive vs Cooperative Multitasking
A multitasking system is one in which CPU time is shared between multiple tasks automatically.

We have two main types of multitasking schedulers:
1. **cooperative**: a process runs until it **voluntarily yield** the CPU by calling a yield syscall. Common in early OSes but not today (by itself) because a malicious or buggy process can take control of the system by never yielding. 
2. **preemptive**: the kernel assures himself CPU time by configuring an hardware timer interrupt to fire periodically so that it can schedule the next process.

>Learnix is a **non-preemptive kernel** (because kernel code cannot be interrupted) that uses a **preemptive scheduler** since it configures the hardware timer interrupt (LAPIC on x86_64) to fire periodically.

### Scheduling metrics
Some important scheduling metrics are:
- **throughput**: total amount of work completed per time unit
- **turnaround time**: total time from a process's arrival to its completion
- **response time**: the time from when the process arrives to the first time it is scheduled
- **fairness**: does every process get a reasonable share of the CPU?
- **starvation**: can a low priority process be permanently denied of CPU time? 

Different computing workloads demand to maximise different scheduling metrics, for example:
- a desktop OS wants minimum response time at the expense of throughput
- a server wants maximum throughput and doesn't really care about response time

## Learnix's Implementation
Learnix employs a strict separation between:
- **mechanism**: how to switch between processes, implemented by `schedule()`
- **policy**: how to decide who and when to switch, implemented by each algorithm in `sched_pick_next()` and `sched_tick()`

Just before popping the saved general purpose registers when returning from an interrupt we check if the current process's has the `PROC_NEED_RESCHED` flag set (equivalent to Linux's `TIF_NEED_RESCHED`).
If so we call `schedule()` to pick the next process that should be run and then we context switch to it by calling `arch_context_switch()`. 

### Interface
Those are the functions that you need to implement for a new scheduling algorithm:
- `sched_init()`: use this to allocate internal data structures and to define the initial state of the algorithm
- `sched_tick()`: let's you define what to do when a timer interrupt fires
- `sched_insert_proc()`: add a process to the runqueue
- `sched_remove_proc()`: remove a process from the runqueue
- `sched_pick_next()`: called by `schedule()`, returns the next process that should run

Have a look at the already supported ones in the next section also.

### Already supported algorithms
The Learnix kernel already implemnts the following scheduling algorithms:
- [Round Robin](rr/README.md)
- [Lottery](lottery/README.md)
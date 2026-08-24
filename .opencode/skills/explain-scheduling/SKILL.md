---
name: explain-scheduling
description: Use when explaining the LearnixOS scheduler and process lifecycle - how processes are chosen to run (mechanism vs policy), context switching, round-robin vs lottery, preemptive timer interrupts, or the process states (READY/RUNNING/WAITING/ZOMBIE). Source of truth is kernel/sched/README.md, sched/rr, sched/lottery, and proc/README.md.
---

# Explaining LearnixOS Scheduling and Processes

Scheduling and processes are the heartbeat of LearnixOS. Use this skill to
explain *who runs*, *how they are swapped*, and *how they live and die*.

## Process lifecycle (from `kernel/proc/README.md`)
- **PCB** = `struct process` (kernel/proc/process.c) - pid, state, `kstack`,
  `pgtable`, `tf`, `ctx`, `parent`, `fds[]`, scheduler fields.
- `procs[64]` table; pid is the index (O(1) lookup).
- **States**: `CREATED → READY → RUNNING ↔ (WAITING | READY)` and `→ ZOMBIE`
  (reaped by parent's `wait()`).
- **IDLE = PID 0** (runqueue sentinel), **INIT = PID 1** (first user process).
- `fork()` copies everything (user address space, fds, trap frame); `execve()`
  tears the old image down and loads a new ELF; `exit()` reparents children to
  PID 1 and becomes a ZOMBIE; `wait()` reaps and frees a ZOMBIE child.

## Scheduler mechanism vs policy
The core abstraction (`kernel/sched/README.md`): separate *mechanism* from
*policy*.
- **Mechanism** = switching between processes = `schedule()`, which reads the
  `PROC_NEED_RESCHED` flag (like Linux's `TIF_NEED_RESCHED`) just before
  returning from an interrupt and calls `arch_context_switch()`.
- **Policy** = who to switch to = per-algorithm `sched_tick()`,
  `sched_pick_next()`, `sched_insert/remove_proc()`.
- New algorithm = implement that interface, select with `-Dsched=<name>`.

## Preemption
Learnix configures the **LAPIC timer** (vector 0x20, see `arch/amd64/idt.c`)
to fire periodically and call `sched_tick()`. So the scheduler is *preemptive*
even though kernel code itself runs with interrupts off (non-reentrant).

## The two built-ins
- **Round Robin** (`kernel/sched/rr/README.md`): circular doubly-linked list;
  idle process as sentinel; each runnable proc gets one fixed **quantum**;
  simple, fair, starvation-free.
- **Lottery** (`kernel/sched/lottery/README.md`): proportional-share; processes
  with more **tickets** win the random "lottery" more often, so importance maps
  to CPU share.

## Context switch (tie to arch)
`arch_context_switch()` (see `explain-boot`/arch README) changes the TSS
`rsp0`, swaps `CR3` (flushing the TLB) and switches kernel stacks via the asm
`_switch_to`. This is the *mechanism* the scheduler drives.

## Common follow-ups
- "Why a quantum?" - response time vs throughput.
- "Cooperative vs preemptive" - yield() syscall vs LAPIC timer.
- "How does init reap?" - init loops calling `wait()` to free ZOMBIE children.

## Answering
Read `kernel/sched/README.md` (plus the `rr/` and `lottery/` READMEs) and
`kernel/proc/README.md`. For a question about a particular algorithm, read
that directory's `.c` and map each required interface function to how the
algorithm implements it.
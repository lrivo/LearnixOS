#pragma once

/*
 * Bootstraps the absolute minimum CPU state to catch fatal exceptions
 * early on before the memory managers are operational.
 * We want a page fault handler while initializing virtual memory.
 *
 * NOTE: must be called with interrupts off.
 */
void arch_stage_1 ();

/*
 * Fully initializes the CPU for running processes
 * and (in the future) SMP multi-core.
 */
void arch_stage_2 ();

/*
 * Indefinitely halts the CPU.
 */
void arch_hcf ();

/*
 * HAL function signatures, which are implemented under kernel/arch
 */
#pragma once

/*
 * @brief bootstraps the absolute minimum CPU state to catch fatal exceptions
 * early on before the memory managers are operational.
 * We want a page fault handler while initializing virtual memory.
 *
 * @note for example in x86_64 we initialize the GDT and an exception-only IDT.
 *
 * @warning interrupts must be disabled.
 */
void arch_stage_1();

/*
 * @brief fully initializes the CPU for running processes and (in the future)
 * SMP.
 */
void arch_stage_2();

/*
 * @brief indefinitely halts the CPU.
 */
void arch_hcf();

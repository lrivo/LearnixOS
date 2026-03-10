/*
 * Architecture indipendend interrupt functions.
 */
#pragma once
#include <stddef.h>

/**
 * @brief placeholder definition that each architecture must define and pass to his interrupt handlers.
 */
struct intr_stack_frame_t;

typedef void (*intr_handler_t)(struct intr_stack_frame_t *f);

/**
 * @brief additional flags (that are translated for the target architecture)
 */
typedef enum {
  INTR_FLAG_DEFAULT = 0,  /* kernel privilege, disables other interrupts */
  INTR_FLAG_USR = 1,      /* callable by userspace */
  INTR_FLAG_TRAP = 2,     /* don´t disable other interrupts, x86 trap gate */
} intr_flags_t;

/**
 * @brief maps the given interrupt vector to a generic C handler function.
 */
void arch_interrupts_register(size_t vector, intr_handler_t handler, intr_flags_t flags);

/**
 * @brief globally enables maskable hardware interrupts.
 * 
 * @note for inlining to work properly it must be directly in the header file. 
 */
static inline void arch_interrupts_enable(void) {
  #if defined(__x86_64__)
    asm volatile("sti" : : : "memory");
  #else
    #error "Unsupported architecture"
  #endif
}

/**
 * @brief globally disables maskable hardware interrupts.
 *
 * @note for inlining to work properly it must be directly in the header file. 
 */
static inline void arch_interrupts_disable(void) {
  #if defined(__x86_64__)
    asm volatile("cli" : : : "memory");
  #else
    #error "Unsupported architecture"
  #endif
}

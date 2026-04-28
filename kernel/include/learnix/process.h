#pragma once
#include <learnix/arch/interrupts.h>
#include <learnix/arch/process.h>
#include <learnix/types.h>

#define KSTACK_SIZE 4096

enum proc_state
{
  RUNNING, /* currently on the CPU */
  READY,   /* can run but CPU is busy */
  WAITING, /* suspended for I/O or something else */
  ZOMBIE,  /* has called exit() but hasn't been fully deallocated. */
};

/* Process Control Block structure. */
struct process
{
  /* general information on the process */
  pid_t pid;
  enum proc_state state;

  /* process kernel stack */
  void *kstack;

  /* page table root of this process */
  void *pgtable;

  /* points to the interrupt struct frame on
   * the kernel stack (architecture-specific) */
  struct intr_trap_frame *tf;

  /* used by arch_switch_to() to context-switch. */
  struct arch_proc_context ctx;

  /* scheduling information */
  int priority;
  void *sched_data;
};

/* Initializes the idle kernel process (the first one) */
void proc_init (void);

/* Initializes a new empty process with minimal state (es: kernel's pgtable
 * copy). */
struct process *proc_create (void);

/* Destroys (if possible) the given process */
void proc_destroy (struct process *p);

struct process *proc_by_pid (pid_t pid);

// ===== ARCH DEPENDENT CODE ====== //
/* Build the initial kernel stack state for a new process */
void arch_proc_init (struct process *p, vaddr_t user_ip, vaddr_t user_sp);

/* Does context-switch from prev to next */
void arch_context_switch (struct process *prev, struct process *next);

void arch_copyuvm (struct process *parent, struct process *child);

void arch_test_jump_usermode (struct intr_trap_frame *tf);

#pragma once
#include <learnix/sched_types.h>
#include <learnix/arch/interrupts.h>
#include <learnix/arch/process.h>
#include <learnix/mm/vma.h>
#include <learnix/fs.h>

#define NFDS 10
#define PROCS_LEN 64 
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
  vaddr_t kstack;

  /* page table root of this process */
  vaddr_t pgtable;

  /* points to the interrupt struct frame on
   * the kernel stack (architecture-specific) */
  struct intr_trap_frame *tf;

  /* used by arch_switch_to() to context-switch. */
  struct arch_proc_context ctx;
  
  /* needed by the wait syscall. */
  struct process* parent;
  struct wait_queue *child_wq;
  
  /* file descriptors table for this process */
  struct file *fds[NFDS];
  
  /* virtual memory areas for this process */
  struct vm_area *vma_head;
  
  /* scheduling */
  struct sched_stats sched_stats; // policy-independent statistics
  uint64_t priority;
  uint64_t ticks_left;
  /* runqueue */
  struct process *next;
  struct process *prev;
};

/* Initializes the idle kernel process (the first one) */
void proc_init (void);

/* Initializes a new empty process with minimal state (es: kernel's pgtable
 * copy). */
struct process *proc_create (void);

/* Destroys (if possible) the given process */
void proc_destroy (struct process *p);

/* Returns the PCB struct for a given pid. */
struct process *proc_by_pid (pid_t pid);

// ===== ARCH DEPENDENT CODE ====== //
/* Build the initial kernel stack state for a new process */
void arch_proc_init (struct process *p, vaddr_t user_ip, vaddr_t user_sp);

/* Edits the kernel stack trap frame for execve. */
void arch_proc_exec (struct process *p, vaddr_t user_ip, vaddr_t user_sp);

/* Does context-switch from prev to next */
void arch_context_switch (struct process *prev, struct process *next);

void arch_test_jump_usermode (struct intr_trap_frame *tf);

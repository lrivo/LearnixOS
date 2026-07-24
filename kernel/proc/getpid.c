#include <learnix/cpu.h>
#include <learnix/process.h>
#include <learnix/syscall.h>

/* Returns the PID of the calling process. Never fails. */
void
sys_getpid (struct intr_trap_frame *tf) {
  RET (tf) = arch_cpu_get ()->proc->pid;
}

/* Returns the PID of the parent of the calling process. Never fails. */
void
sys_getppid(struct intr_trap_frame *tf) {
    RET(tf) = arch_cpu_get()->proc->parent->pid;
}

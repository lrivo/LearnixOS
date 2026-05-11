#include <learnix/cpu.h>
#include <learnix/process.h>
#include <learnix/syscall.h>

void
sys_getpid (struct intr_trap_frame *tf)
{
  RET (tf) = arch_cpu_get ()->proc->pid;
}

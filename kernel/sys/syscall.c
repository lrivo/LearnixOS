#include <learnix/cpu.h>
#include <learnix/lib/kprintf.h>
#include <learnix/scheduler.h>
#include <learnix/syscall.h>

void
syscall_dispatcher (struct intr_trap_frame *tf)
{
  size_t num = NUM (tf);
  switch (num)
  {
  case SYS_WRITE:
    sys_write (tf);
    break;
  case SYS_FORK:
    sys_fork (tf);
    break;
  case SYS_EXECVE:
    sys_execve (tf);
    break;
  case SYS_EXIT:
    sys_exit (tf);
    break;
  default:
    tf->rax = -1;
    break;
  }
}

void
sys_write (struct intr_trap_frame *tf)
{
  char *buf = (char *)ARG1 (tf);
  size_t count = (size_t)ARG2 (tf);

  // FIXME: not validating user pointer
  for (size_t i = 0; i < count; i++)
    kprintf ("%c", buf[i]);

  // write() should return the number of written chars
  tf->rax = count;
}

/* for now we just mark the current process
   as zombie and immediately schedule to the next
   runnable process. */
void
sys_exit (struct intr_trap_frame *tf)
{
  (void)tf;

  arch_cpu_get ()->proc->state = ZOMBIE;

  schedule ();

  // will never return here
}

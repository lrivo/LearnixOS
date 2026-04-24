#include "learnix/arch/syscall.h"
#include <learnix/lib/kprintf.h>
#include <learnix/syscall.h>

void
syscall_dispatcher (struct sys_trap_frame *tf)
{
  size_t num = NUM (tf);
  switch (num)
  {
  case SYS_WRITE:
    sys_write (tf);
    break;
  default:
    break;
  }
}

void
sys_write (struct sys_trap_frame *tf)
{
  char *buf = (char *)ARG1 (tf);
  size_t count = (size_t)ARG2 (tf);

  // FIXME: not validating user pointer
  for (size_t i = 0; i < count; i++)
    kprintf ("%c", buf[i]);

  // write() should return the number of written chars
  tf->rax = count;
}

#include "learnix/arch/types.h"
#include <learnix/drivers/console/console.h>
#include <learnix/lib/string.h>
#include <learnix/mm/kmalloc.h>
#include <learnix/syscall.h>

ssize_t
sys_write (uint32_t fd, const char *buf, size_t count)
{
  // FIXME: super bad, a process can just dump kernel memory
  for (size_t i = 0; i < count; i++)
    console_putchar (buf[i]);
  return count;
}

ssize_t
syscall_dispatcher (size_t num, size_t arg0, size_t arg1, size_t arg2,
                    size_t arg3, size_t arg4, size_t arg5)
{
  switch (num)
  {
  case 1:
  {
    return sys_write ((uint32_t)arg0, (const char *)arg1, arg2);
  }
  default:
    break;
  }
  return 0;
}

#include "learnix/lib/kprintf.h"
#include <learnix/syscall.h>

ssize_t
syscall_dispatcher (size_t num, size_t arg0, size_t arg1, size_t arg2,
                    size_t arg3, size_t arg4, size_t arg5)
{
  kprintf ("syscall_dispatcher: %lu\n", num);
  return 0;
}

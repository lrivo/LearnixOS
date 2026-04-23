#include "types.h"

struct arch_cpu_data
{
  /* SYSCALL */
  uint64_t usr_rsp; /* scratch space for saving userland RSP */
};

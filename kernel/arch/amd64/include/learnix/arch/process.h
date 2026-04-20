#pragma once
#include "types.h"

/* This is what switch_to() uses to context-switch */
struct arch_proc_context
{
  uint64_t rsp;
};

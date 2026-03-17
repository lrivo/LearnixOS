#pragma once

#include <stddef.h>

/* Prints at most max_frames stack frames from the current rbp */
void dbg_print_stack_trace(size_t max_frames);

/* Dumps n bytes of memory in hex starting from va. */
void dbg_hexdump(void *va, size_t n);

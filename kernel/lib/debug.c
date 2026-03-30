#include "lib/kprintf.h"
#include "mm/memlayout.h"
#include <lib/debug.h>
#include <stddef.h>
#include <stdint.h>

struct stackframe
{
  struct stackframe *rbp;
  uintptr_t ret;
};

/* We can walk the stack frame by recursively reading the saved base pointer. */
void
dbg_print_stack_trace (size_t max_frames)
{
  // this returns the current base pointer address
  struct stackframe *frame = __builtin_frame_address (0);

  for (size_t i = 0; i < max_frames; i++)
  {
    // kmain() will always have the saved rbp to 0
    if (!frame)
      break;
    if ((uintptr_t)frame->rbp < hhdm_offset)
      break;

    // print the return address of this frame
    kprintf ("[%zu] %p\n", i, frame->ret);

    // go to the next stack frame
    frame = frame->rbp;
  }
}

void
dbg_hexdump (void *va, size_t n)
{
  uint8_t *p = (uint8_t *)va;

  for (size_t i = 0; i < n * 16; i += 16)
  {
    // print the row's virtual address
    kprintf ("%016lx: ", (uintptr_t)p + i);

    // print row bytes one by one
    for (size_t j = 0; j < 16; j++)
    {
      kprintf ("%02x ", p[i + j]);
    }
    kprintf (" | ");

    // print row bytes as ASCII one by one
    for (size_t j = 0; j < 16; j++)
    {
      char c = (char)p[i + j];
      if (c >= 32 && c <= 126)
      {
        kprintf ("%c", c);
      }
      else
      {
        kprintf (".");
      }
    }
    kprintf (" |\n");
  }
}

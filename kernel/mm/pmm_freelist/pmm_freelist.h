#pragma once
#include <learnix/types.h>

#define PG_ALLOC  (1 << 0U)     // if set the page is allocated
#define PG_PINNED (1 << 1U)     // if set the page cannot be allocated/free-ed (kernel code, bootloader reserved)

struct page
{
  uint32_t flags;
  uint32_t refcount;
  struct page *next;
  struct page *prev;
};
